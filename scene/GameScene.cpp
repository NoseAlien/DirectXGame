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

	billBoardParentTransform_.translation_ = { 0,0,0 };
	billBoardParentTransform_.rotation_ = { 0,0,0 };
	billBoardParentTransform_.scale_ = { 7,1,1 };
	billBoardParentTransform_.Initialize();

	billBoardTransform_.translation_ = { 0,0,0 };
	billBoardTransform_.rotation_ = { 0,0,0 };
	billBoardTransform_.scale_ = { 1,1,0.001f };
	billBoardTransform_.parent_ = &billBoardParentTransform_;
	billBoardTransform_.Initialize();

	cameraTransform_.translation_ = { 0,0,-40 };
	cameraTransform_.rotation_ = { 0,0,0 };
	cameraTransform_.scale_ = { 1,1,1 };
	cameraTransform_.Initialize();

	viewProjection_.eye = { 0,0,0 };
	viewProjection_.target = { 0,0,1 };
	viewProjection_.up = { 0,1,0 };
	viewProjection_.Initialize();
}

void GameScene::Update() {

	//視点移動
	XMFLOAT3 move = { 0,0,0 };
	XMFLOAT3 cameraRight = getRelativeDirection(cameraTransform_, { 1,0,0 });
	XMFLOAT3 cameraUp = getRelativeDirection(cameraTransform_, { 0,1,0 });
	XMFLOAT3 cameraFront = getRelativeDirection(cameraTransform_, { 0,0,1 });

	const float kEyeSpeed = 0.2f;

	if (input_->PushKey(DIK_W))
	{
		move = { cameraFront.x * kEyeSpeed,cameraFront.y * kEyeSpeed ,cameraFront.z * kEyeSpeed };
	}
	else if (input_->PushKey(DIK_S))
	{
		move = { cameraFront.x * -kEyeSpeed,cameraFront.y * -kEyeSpeed ,cameraFront.z * -kEyeSpeed };
	}

	if (input_->PushKey(DIK_D))
	{
		move = { cameraRight.x * kEyeSpeed,cameraRight.y * kEyeSpeed ,cameraRight.z * kEyeSpeed };
	}
	else if (input_->PushKey(DIK_A))
	{
		move = { cameraRight.x * -kEyeSpeed,cameraRight.y * -kEyeSpeed ,cameraRight.z * -kEyeSpeed };
	}

	if (input_->PushKey(DIK_SPACE))
	{
		move = { cameraUp.x * kEyeSpeed,cameraUp.y * kEyeSpeed ,cameraUp.z * kEyeSpeed };
	}
	else if (input_->PushKey(DIK_LSHIFT))
	{
		move = { cameraUp.x * -kEyeSpeed,cameraUp.y * -kEyeSpeed ,cameraUp.z * -kEyeSpeed };
	}

	cameraTransform_.translation_.x += move.x;
	cameraTransform_.translation_.y += move.y;
	cameraTransform_.translation_.z += move.z;

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
	if (input_->PushKey(DIK_UP))
	{
		move.x -= 0.01f;

	}
	else if (input_->PushKey(DIK_DOWN))
	{
		move.x += 0.01f;

	}
	if (input_->PushKey(DIK_E))
	{
		move.z -= 0.01f;

	}

	cameraTransform_.rotation_.x += move.x;
	cameraTransform_.rotation_.y += move.y;
	cameraTransform_.rotation_.z += move.z;

	//カメラの視点と向きをcameraTransform_に合わせる
	viewProjection_.eye = cameraTransform_.translation_;

	viewProjection_.target = {
		cameraTransform_.translation_.x + getRelativeDirection(cameraTransform_,{0,0,1}).x
		,cameraTransform_.translation_.y + getRelativeDirection(cameraTransform_,{0,0,1}).y
		,cameraTransform_.translation_.z + getRelativeDirection(cameraTransform_,{0,0,1}).z };

	viewProjection_.up = getRelativeDirection(cameraTransform_, { 0,1,0 });

	//行列の再計算
	viewProjection_.UpdateMatrix();

	//ここからビルボード処理

	billBoardTransform_.rotation_ = {
		-cameraTransform_.rotation_.x
		,cameraTransform_.rotation_.y + XM_PI
		,-cameraTransform_.rotation_.z};

	billBoardTransform_.UpdateMatrix();

	//ここまでビルボード処理

	//デバッグ表示
	debugText_->SetPos(50, 50);
	debugText_->Printf(
		"eye:(%f,%f,%f)"
		, viewProjection_.eye.x, viewProjection_.eye.y, viewProjection_.eye.z);
	debugText_->SetPos(50, 70);
	debugText_->Printf(
		"target:(%f,%f,%f)"
		, viewProjection_.target.x, viewProjection_.target.y, viewProjection_.target.z);
	debugText_->SetPos(50, 90);
	debugText_->Printf(
		"up:(%f,%f,%f)"
		, viewProjection_.up.x, viewProjection_.up.y, viewProjection_.up.z);
	debugText_->SetPos(50, 110);
	debugText_->Printf(
		"camRot:(%f,%f,%f)"
		, cameraTransform_.rotation_.x, cameraTransform_.rotation_.y, cameraTransform_.rotation_.z);
	debugText_->SetPos(50, 130);
	debugText_->Printf(
		"billboardRot:(%f,%f,%f)"
		, billBoardTransform_.rotation_.x, billBoardTransform_.rotation_.y, billBoardTransform_.rotation_.z);
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

	model_->Draw(billBoardTransform_, viewProjection_, textureHandle2_);

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
