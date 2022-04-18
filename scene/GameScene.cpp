#include "GameScene.h"
#include "TextureManager.h"
#include <cassert>
#include <random>

using namespace DirectX;

GameScene::GameScene() {}

GameScene::~GameScene() {
	delete model_;
}

void GameScene::Initialize() {

	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();
	debugText_ = DebugText::GetInstance();

	//ファイル名を指定してテクスチャを読み込む
	textureHandle_ = TextureManager::Load("napnose.png");
	
	model_ = Model::Create();

	std::random_device seed_gen;
	std::mt19937_64 engine(seed_gen());
	std::uniform_real_distribution<float> rotDist(0.0f, XM_2PI);
	std::uniform_real_distribution<float> posDist(-10.0f,10.0f);

	for (size_t i = 0; i < _countof(worldTransform_); i++)
	{
		worldTransform_[i].translation_ = {posDist(engine),posDist(engine),posDist(engine) };
		worldTransform_[i].rotation_ = { rotDist(engine),rotDist(engine),rotDist(engine) };
		worldTransform_[i].scale_ = { 1,1,1 };
		worldTransform_[i].Initialize();
	}

	viewProjection_.eye = { 0,0,-10 };
	viewProjection_.target = { 10,0,0 };
	viewProjection_.Initialize();
}

void GameScene::Update() {

	//視点移動
	XMFLOAT3 move = { 0,0,0 };

	const float kEyeSpeed = 0.2f;

	if (input_->PushKey(DIK_W))
	{
		move = { 0,0,kEyeSpeed };
	}
	else if (input_->PushKey(DIK_S))
	{
		move = { 0,0,-kEyeSpeed };
	}

	viewProjection_.eye.x += move.x;
	viewProjection_.eye.y += move.y;
	viewProjection_.eye.z += move.z;

	//注視点移動
	move = { 0,0,0 };

	const float kTargetSpeed = 0.2f;

	if (input_->PushKey(DIK_LEFT))
	{
		move = { -kTargetSpeed,0,0 };
	}
	else if (input_->PushKey(DIK_RIGHT))
	{
		move = { kTargetSpeed,0,0 };
	}

	viewProjection_.target.x += move.x;
	viewProjection_.target.y += move.y;
	viewProjection_.target.z += move.z;

	//上方向回転
	const float kUpRotSpeed = 0.05f;

	if (input_->PushKey(DIK_SPACE))
	{
		viewAngle += kUpRotSpeed;
		viewAngle = fmodf(viewAngle,XM_2PI);
	}

	viewProjection_.up = { cosf(viewAngle), sinf(viewAngle),0 };

	//行列の再計算
	viewProjection_.UpdateMatrix();

	//デバッグ表示
	debugText_->SetPos(50, 50);
	debugText_->Printf(
		"eye:(%f,%f,%f)"
		, viewProjection_.eye.x, viewProjection_.eye.y, viewProjection_.eye.z);
	debugText_->SetPos(50, 70);
	debugText_->Printf(
		"target:(%f,%f,%f)"
		, viewProjection_.target.x, viewProjection_.target.y, viewProjection_.target.z);
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
	for (size_t i = 0; i < _countof(worldTransform_); i++)
	{
		model_->Draw(worldTransform_[i], viewProjection_, textureHandle_);
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
