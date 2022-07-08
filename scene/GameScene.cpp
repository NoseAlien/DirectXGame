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
	delete sprite_;
}

void GameScene::Initialize() {

	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();
	debugText_ = DebugText::GetInstance();

	std::random_device seed_gen;
	std::mt19937_64 engine(seed_gen());
	std::uniform_real_distribution<float> dist(0, MathUtility::PI * 2);

	//ファイル名を取得してテクスチャを読み込む
	textureHandle_ = TextureManager::Load("napnose.png");

	cursorTex = TextureManager::Load("MEGNOSE.png");

	//3Dモデルの生成
	model_ = Model::Create();

	sprite_ = Sprite::Create(cursorTex, { 640,360 });
	sprite_->SetAnchorPoint({0.5,0.5});

	viewProjection_.fovAngleY = DegreeToRad(50);

	viewProjection_.eye = { 0,0,-40 };
	viewProjection_.target = { 0,0,0 };
	viewProjection_.up = { 0,1,0 };
	viewProjection_.Initialize();

	//デバッグカメラの生成
	debugCamera_ = new DebugCamera(400,300);

	/*
	//軸方向の表示を有効にする
	AxisIndicator::GetInstance()->SetVisible(true);
	//軸方向表示が参照するビュープロジェクションを指定する（アドレス渡し）
	AxisIndicator::GetInstance()->SetTargetViewProjection(&viewProjection_);

	//ライン描画が参照するビュープロジェクションを指定する（アドレス渡し）
	PrimitiveDrawer::GetInstance()->SetViewProjection(&debugCamera_->GetViewProjection());
	*/

	//自キャラの生成
	//player_ = new Player();
	//自キャラの初期化
	//player_->Initialize(model_, textureHandle_);

	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			worldTransform_[i][j].Initialize();
			worldTransform_[i][j].translation_ = { ((float)i - 4) * 3,((float)j - 4) * 3,0 };
			worldTransform_[i][j].scale_ = { 1,1,1 };
			worldTransform_[i][j].UpdateMatrix();
		}
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

	Vector3 move = { 0,0,0 };

	if (input_->PushKey(DIK_RIGHT))
	{
		move.x += 0.1;
	}
	if (input_->PushKey(DIK_LEFT))
	{
		move.x -= 0.1;
	}

	if (input_->PushKey(DIK_UP))
	{
		move.y += 0.1;
	}
	if (input_->PushKey(DIK_DOWN))
	{
		move.y -= 0.1;
	}

	viewProjection_.target += move;

	isScopeMode = input_->PushKey(DIK_SPACE);

	int targetFovDegree;
	if (isScopeMode)
	{
		targetFovDegree = 20;
	}
	else
	{
		targetFovDegree = 50;
	}

	if (fovDegree < targetFovDegree)
	{
		fovDegree++;
	}
	if (fovDegree > targetFovDegree)
	{
		fovDegree--;
	}

	viewProjection_.fovAngleY = DegreeToRad(fovDegree);


	if (viewProjection_.fovAngleY < 0)
	{
		viewProjection_.fovAngleY = 0;
	}
	if (viewProjection_.fovAngleY > MathUtility::PI)
	{
		viewProjection_.fovAngleY = MathUtility::PI;
	}

	viewProjection_.UpdateMatrix();

	//player_->Update();

	debugText_->SetPos(50, 50);
	debugText_->Printf(
		"eye:(%f,%f,%f)", viewProjection_.eye.x, viewProjection_.eye.y, viewProjection_.eye.z);
	debugText_->SetPos(50, 70);
	debugText_->Printf(
		"target:(%f,%f,%f)", viewProjection_.target.x, viewProjection_.target.y, viewProjection_.target.z);
	debugText_->SetPos(50, 90);
	debugText_->Printf(
		"up:(%f,%f,%f)", viewProjection_.up.x, viewProjection_.up.y, viewProjection_.up.z);
	debugText_->SetPos(50, 110);
	debugText_->Printf(
		"fovAngleY:%f", viewProjection_.fovAngleY);
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

	for (int i = 0; i < sizeof(worldTransform_) / sizeof(worldTransform_[0]); i++)
	{
		for (int j = 0; j < sizeof(worldTransform_) / sizeof(worldTransform_[0]); j++)
		{
			model_->Draw(worldTransform_[i][j], viewProjection_, textureHandle_);
		}
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
	
	if (isScopeMode)
	{
		sprite_->Draw();
	}

	// デバッグテキストの描画
	debugText_->DrawAll(commandList);
	//
	// スプライト描画後処理
	Sprite::PostDraw();

#pragma endregion
}
