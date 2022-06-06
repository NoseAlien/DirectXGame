#include "GameScene.h"
#include "TextureManager.h"
#include <cassert>
#include "AxisIndicator.h"
#include "PrimitiveDrawer.h"
#include <random>

double DegreeToRad(double num)
{
	return num / 180 * MathUtility::PI;
}

double RadToDegree(double num)
{
	return num / MathUtility::PI * 180;
}

float VectorScale(Vector3 v)
{
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vector3 cross(Vector3 v, Vector3 v2)
{
	return Vector3(v.y * v2.z - v.z * v2.y, v.z * v2.x - v.x * v2.z, v.x * v2.y - v.y * v2.x);
}

Vector3 getRelativeDirection(WorldTransform rotTarget, Vector3 relativeDirection)
{
	rotTarget.UpdateMatrix();
	Matrix4 matWorldRot = rotTarget.matWorldRot_;
	relativeDirection = {
		-relativeDirection.x * matWorldRot.m[0][0] - relativeDirection.y * matWorldRot.m[0][1] - relativeDirection.z * matWorldRot.m[0][2],
		relativeDirection.x * matWorldRot.m[1][0] + relativeDirection.y * matWorldRot.m[1][1] + relativeDirection.z * matWorldRot.m[1][2],
		relativeDirection.x * matWorldRot.m[2][0] + relativeDirection.y * matWorldRot.m[2][1] + relativeDirection.z * matWorldRot.m[2][2]
	};
	return relativeDirection;
}

GameScene::GameScene() {}

GameScene::~GameScene() {
	delete model_;
	delete debugCamera_;
	delete player_;
}

void GameScene::Initialize() {

	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();
	debugText_ = DebugText::GetInstance();

	std::random_device seed_gen;
	std::mt19937_64 engine(seed_gen());
	std::uniform_real_distribution<float> dist(-10.0,10.0);

	std::random_device seed_genRot;
	std::mt19937_64 rotEngine(seed_genRot());
	std::uniform_real_distribution<float> rotDist(0, MathUtility::PI);

	//ファイル名を取得してテクスチャを読み込む
	textureHandle_ = TextureManager::Load("napnose.png");

	//3Dモデルの生成
	model_ = Model::Create();

	viewProjection_.eye = {0,0,-30};
	viewProjection_.target = { 0,0,0 };
	viewProjection_.up = {0,1,0};

	/*viewProjection_.fovAngleY = DegreeToRad(10);
	viewProjection_.aspectRatio = 1;
	viewProjection_.nearZ = 52.0f;
	viewProjection_.farZ = 53.0f;*/

	//ビュープロジェクションの初期化
	viewProjection_.Initialize();

	//デバッグカメラの生成
	debugCamera_ = new DebugCamera(400,300);

	//軸方向の表示を有効にする
	AxisIndicator::GetInstance()->SetVisible(true);
	//軸方向表示が参照するビュープロジェクションを指定する（アドレス渡し）
	AxisIndicator::GetInstance()->SetTargetViewProjection(&viewProjection_);

	//ライン描画が参照するビュープロジェクションを指定する（アドレス渡し）
	PrimitiveDrawer::GetInstance()->SetViewProjection(&debugCamera_->GetViewProjection());

	//自キャラの生成
	player_ = new Player();
	//自キャラの初期化
	player_->Initialize(model_, textureHandle_);

	for (int i = 0; i < 9; i++)
	{
		worldTransforms_[i].Initialize();
		worldTransforms_[i].translation_ = { ((float)i - 4) * 6,10,0 };
		worldTransforms_[i].scale_ = { 3,3,3 };
		worldTransforms_[i].UpdateMatrix();
	}
	for (int i = 9; i < 18; i++)
	{
		worldTransforms_[i].Initialize();
		worldTransforms_[i].translation_ = { ((float)i - 13) * 6,-10,0 };
		worldTransforms_[i].scale_ = { 3,3,3 };
		worldTransforms_[i].UpdateMatrix();
	}

	playable_ = new WorldTransform();

	playable_->translation_ = { 0,0,0 };
	playable_->scale_ = { 1,1,2 };
	playable_->Initialize();
}

void GameScene::Update() {
#ifdef _DEBUG
	if (input_->TriggerKey(DIK_P))
	{
		isDebugCameraActive_ = !isDebugCameraActive_;
	}
#endif

	if (isDebugCameraActive_)
	{
		debugCamera_->Update();
		viewProjection_.matView = debugCamera_->GetViewProjection().matView;
		viewProjection_.matProjection = debugCamera_->GetViewProjection().matProjection;
	}

	if (input_->PushKey(DIK_RIGHT))
	{
		viewAngleY += 0.01;
	}
	if (input_->PushKey(DIK_LEFT))
	{
		viewAngleY -= 0.01;
	}

	viewProjection_.target = viewProjection_.eye;
	viewProjection_.target += { sinf(viewAngleY), 0, cosf(viewAngleY) };

	viewProjection_.UpdateMatrix();

	//player_->Update();

	if (input_->TriggerKey(DIK_Q))
	{
		viewMovement = !viewMovement;
	}

	Vector3 frontVec = { 0,0,0 };
	Vector3 rightVec = { 0,0,0 };

	if (viewMovement)
	{
		frontVec = viewProjection_.target;
		frontVec -= viewProjection_.eye;
		frontVec /= VectorScale(frontVec);

		rightVec = cross(viewProjection_.up, frontVec);
		rightVec /= VectorScale(rightVec);

		debugText_->SetPos(50, 110);
		debugText_->Printf(
			"ViewMove");
	}
	else
	{
		frontVec = getRelativeDirection(*playable_, { 0,0,1 });

		debugText_->SetPos(50, 110);
		debugText_->Printf(
			"RelativeMove");
	}

	Vector3 move = { 0,0,0 };

	if (input_->PushKey(DIK_W))
	{
		move += frontVec;
	}
	if (input_->PushKey(DIK_S))
	{
		move -= frontVec;
	}

	if (viewMovement)
	{
		if (input_->PushKey(DIK_D))
		{
			move += rightVec;
		}
		if (input_->PushKey(DIK_A))
		{
			move -= rightVec;
		}
	}
	else
	{
		if (input_->PushKey(DIK_D))
		{
			playable_->rotation_.y += 0.01;
		}
		if (input_->PushKey(DIK_A))
		{
			playable_->rotation_.y -= 0.01;
		}
	}

	move *= 0.05f;

	playable_->translation_ += move;

	playable_->UpdateMatrix();
}

void GameScene::Draw() {

	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

#pragma region 背景スプライト描画
	// 背景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに背景スプライトの描画処理を追加できる
	/// </summary>

	// スプライト描画後処理
	Sprite::PostDraw();
	// 深度バッファクリア
	dxCommon_->ClearDepthBuffer();
#pragma endregion

#pragma region 3Dオブジェクト描画
	// 3Dオブジェクト描画前処理
	Model::PreDraw(commandList);

	/// <summary>
	/// ここに3Dオブジェクトの描画処理を追加できる
	/// </summary>
	//3Dモデル描画
	
	//player_->Draw(viewProjection_);

	model_->Draw(*playable_, viewProjection_, textureHandle_);

	for (int i = 0; i < sizeof(worldTransforms_) / sizeof(worldTransforms_[0]); i++)
	{
		model_->Draw(worldTransforms_[i], viewProjection_, textureHandle_);
	}

	// 3Dオブジェクト描画後処理
	Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト描画
	// 前景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに前景スプライトの描画処理を追加できる
	/// </summary>

	// デバッグテキストの描画
	debugText_->DrawAll(commandList);
	//
	// スプライト描画後処理
	Sprite::PostDraw();

#pragma endregion
}
