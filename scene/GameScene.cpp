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
	XMStoreFloat3(&relativeDirection, XMVector3Transform(XMLoadFloat3(&relativeDirection), rotTarget.matWorldRot_));
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
	std::uniform_real_distribution<float> posDist(-10.0f, 10.0f);

	for (size_t i = 0; i < _countof(worldTransform_); i++)
	{
		worldTransform_[i].translation_ = { -10 + float(int(i) / 10) * 2,-10,-10 + float(int(i) % 10) * 2 };
		worldTransform_[i].rotation_ = { 0,0,0 };
		worldTransform_[i].scale_ = { 1,1,1 };
		worldTransform_[i].Initialize();
	}

	targetTransform_[0].translation_ = { 0,10,0 };
	targetTransform_[0].rotation_ = { 0,0,0 };
	targetTransform_[0].scale_ = { 1,1,1 };
	targetTransform_[0].Initialize();

	targetTransform_[1].translation_ = { -5,0,0 };
	targetTransform_[1].rotation_ = { 0,0,0 };
	targetTransform_[1].scale_ = { 1,1,1 };
	targetTransform_[1].Initialize();

	targetTransform_[2].translation_ = { 5,0,0 };
	targetTransform_[2].rotation_ = { 0,0,0 };
	targetTransform_[2].scale_ = { 1,1,1 };
	targetTransform_[2].Initialize();

	viewProjection_.eye = { 0,0,0 };
	viewProjection_.target = { 0,0,1 };
	viewProjection_.up = { 0,1,0 };
	viewProjection_.Initialize();
}

void GameScene::Update() {

	if (input_->PushKey(DIK_SPACE) && !trig)
	{
		targetNum++;
		targetNum = targetNum % _countof(targetTransform_);
	}
	trig = input_->PushKey(DIK_SPACE);

	viewProjection_.eye = { 0,0,-30 };

	viewProjection_.target = targetTransform_[targetNum].translation_;

	viewProjection_.up = { 0,1,0 };

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
	for (size_t i = 0; i < _countof(targetTransform_); i++)
	{
		model_->Draw(targetTransform_[i], viewProjection_, textureHandle_);
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
