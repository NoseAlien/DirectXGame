#include "GameScene.h"
#include "TextureManager.h"
#include <cassert>
#include <random>

using namespace DirectX;

GameScene::GameScene() {}

GameScene::~GameScene() {
	delete model_;
}

XMFLOAT3 getRelativeDirection(WorldTransform rotTarget, XMFLOAT3 relativeDirection)
{
	rotTarget.UpdateMatrix();
	XMStoreFloat3(&relativeDirection,XMVector3Transform(XMLoadFloat3(&relativeDirection), rotTarget.matWorldRot_));
	return relativeDirection;
}

void GameScene::Initialize() {

	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();
	debugText_ = DebugText::GetInstance();

	//ファイル名を指定してテクスチャを読み込む
	textureHandle_ = TextureManager::Load("napnose.png");
	textureHandle2_ = TextureManager::Load("laser.png");
	
	model_ = Model::Create();

	std::random_device seed_gen;
	std::mt19937_64 engine(seed_gen());
	std::uniform_real_distribution<float> rotDist(0.0f, XM_2PI);
	std::uniform_real_distribution<float> posDist(-10.0f,10.0f);

	for (size_t i = 0; i < _countof(worldTransform_); i++)
	{
		worldTransform_[i].translation_ = {-10 + float(int(i) / 10) * 2,-10,-10 + float(int(i) % 10) * 2 };
		worldTransform_[i].rotation_ = { 0,0,0 };
		worldTransform_[i].scale_ = { 1,1,1 };
		worldTransform_[i].Initialize();
	}

	playerTransform_.translation_ = { 0,0,0 };
	playerTransform_.rotation_ = { 0,0,0 };
	playerTransform_.scale_ = { 1,1,2 };
	playerTransform_.Initialize();

	viewProjection_.eye = { 0,0,0 };
	viewProjection_.target = { 0,0,1 };
	viewProjection_.up = { 0,1,0 };
	viewProjection_.Initialize();
}

void GameScene::Update() {

	//視点移動
	XMFLOAT3 move = { 0,0,0 };
	XMFLOAT3 Front = getRelativeDirection(playerTransform_, { 0,0,1 });

	const float kEyeSpeed = 0.2f;

	if (input_->PushKey(DIK_UP))
	{
		move = { Front.x * kEyeSpeed,Front.y * kEyeSpeed ,Front.z * kEyeSpeed };
	}
	else if (input_->PushKey(DIK_DOWN))
	{
		move = { Front.x * -kEyeSpeed,Front.y * -kEyeSpeed ,Front.z * -kEyeSpeed };
	}

	if (input_->PushKey(DIK_SPACE))
	{
	}

	playerTransform_.translation_.x += move.x;
	playerTransform_.translation_.y += move.y;
	playerTransform_.translation_.z += move.z;

	//注視点移動
	move = { 0,0,0 };

	const float kTargetSpeed = 0.2f;

	if (input_->PushKey(DIK_LEFT))
	{
		move.y -= 0.01f;
	}
	else if (input_->PushKey(DIK_RIGHT))
	{
		move.y += 0.01f;
	}

	playerTransform_.rotation_.x += move.x;
	playerTransform_.rotation_.y += move.y;
	playerTransform_.rotation_.z += move.z;

	//カメラの視点と向きをcameraTransform_に合わせる
	XMFLOAT3 camPosition = getRelativeDirection(playerTransform_, { 0,30,-30 });

	viewProjection_.eye = { playerTransform_.translation_.x + camPosition.x
		,playerTransform_.translation_.y + camPosition.y
		,playerTransform_.translation_.z + camPosition.z };

	viewProjection_.target = playerTransform_.translation_;

	viewProjection_.up = getRelativeDirection(playerTransform_, { 0,1,0 });

	//行列の再計算
	viewProjection_.UpdateMatrix();
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

	model_->Draw(playerTransform_, viewProjection_, textureHandle_);

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
