#include "GameScene.h"
#include "TextureManager.h"
#include <cassert>
#include "AxisIndicator.h"
#include "PrimitiveDrawer.h"
#include <random>

GameScene::GameScene() {}

GameScene::~GameScene() {
	delete model_;
	delete debugCamera_;
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

	for (WorldTransform& worldTransform : worldTransforms_)
	{
		//ワールドトランスフォームの初期化
		worldTransform.Initialize();
		worldTransform.scale_ = { 1,1,1 };
		worldTransform.rotation_ = { rotDist(rotEngine),rotDist(rotEngine),rotDist(rotEngine) };
		worldTransform.translation_ = { dist(engine),dist(engine),dist(engine) };
		worldTransform.UpdateMatrix();
	}

	viewProjection_.eye = {0,0,-10};
	viewProjection_.target = { 10,0,0 };
	viewProjection_.up = { cosf(MathUtility::PI / 4),sinf(MathUtility::PI / 4),0};

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
}

void GameScene::Update() {
	debugCamera_->Update();

	Vector3 move = { 0,0,0 };
	const float kEyeSpeed = 0.2f;
	if (input_->PushKey(DIK_W))
	{
		move.z += kEyeSpeed;
	}
	else if (input_->PushKey(DIK_S))
	{
		move.z -= kEyeSpeed;
	}
	viewProjection_.eye += move;

	move = { 0,0,0 };
	const float kTargetSpeed = 0.2f;
	if (input_->PushKey(DIK_LEFT))
	{
		move.x += kTargetSpeed;
	}
	else if (input_->PushKey(DIK_RIGHT))
	{
		move.x -= kTargetSpeed;
	}
	viewProjection_.target += move;

	const float kUpRotSpeed = 0.05f;
	if (input_->PushKey(DIK_SPACE))
	{
		viewAngle += kUpRotSpeed;
		viewAngle = fmodf(viewAngle, MathUtility::PI * 2.0f);
	}
	viewProjection_.up = { cosf(viewAngle),sinf(viewAngle),0 };

	viewProjection_.UpdateMatrix();

	debugText_->SetPos(50, 50);
	debugText_->Printf(
		"eye:(%f,%f,%f)", viewProjection_.eye.x, viewProjection_.eye.y, viewProjection_.eye.z);
	debugText_->SetPos(50, 70);
	debugText_->Printf(
		"target:(%f,%f,%f)", viewProjection_.target.x, viewProjection_.target.y, viewProjection_.target.z);
	debugText_->SetPos(50, 90);
	debugText_->Printf(
		"up:(%f,%f,%f)", viewProjection_.up.x, viewProjection_.up.y, viewProjection_.up.z);
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
	for (WorldTransform& worldTransform : worldTransforms_)
	{
		model_->Draw(worldTransform, viewProjection_, textureHandle_);
	}

	//PrimitiveDrawer::GetInstance()->DrawLine3d(Vector3{ 10,0,0 }, Vector3{ -10,0,0 }, Vector4{ 255,0,0,255 });
	//読み取りアクセス違反。this->line_._Mypair.**_Myval2** が nullptr でした。

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
