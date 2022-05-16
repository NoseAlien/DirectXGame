#include "GameScene.h"
#include "TextureManager.h"
#include <cassert>
#include "AxisIndicator.h"
#include "PrimitiveDrawer.h"

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

	//ファイル名を取得してテクスチャを読み込む
	textureHandle_ = TextureManager::Load("napnose.png");

	//3Dモデルの生成
	model_ = Model::Create();

	//ワールドトランスフォームの初期化
	worldTransform_.Initialize();

	//////拡縮//////
	//X,Y,Z方向のスケーリングを設定
	worldTransform_.scale_ = { 1.5,1,0.5 };
	//スケーリング倍率を行列に設定する
	Matrix4 matScale =
		{ worldTransform_.scale_.x,0,0,0,
		0,worldTransform_.scale_.y,0,0,
		0,0,worldTransform_.scale_.z,0,
		0,0,0,1 };

	//単位行列を代入
	worldTransform_.matWorld_ = 
		{ 1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1 };

	//////回転//////
	//X,Y,Z軸周りの回転角を設定
	worldTransform_.rotation_ = { 0,0,MathUtility::PI / 4 };
	//Z軸回転行列を宣言
	Matrix4 matRotZ = 
		{ cos(worldTransform_.rotation_.z),sin(worldTransform_.rotation_.z),0,0,
		-sin(worldTransform_.rotation_.z),cos(worldTransform_.rotation_.z),0,0,
		0,0,1,0,
		0,0,0,1 };
	//X軸回転行列を宣言
	Matrix4 matRotX =
		{ 1,0,0,0,
		0,cos(worldTransform_.rotation_.x),sin(worldTransform_.rotation_.x),0,
		0,-sin(worldTransform_.rotation_.x),cos(worldTransform_.rotation_.x),0,
		0,0,0,1 };
	//Y軸回転行列を宣言
	Matrix4 matRotY =
		{ cos(worldTransform_.rotation_.y),0,-sin(worldTransform_.rotation_.y),0,
		0,1,0,0,
		sin(worldTransform_.rotation_.y),0,cos(worldTransform_.rotation_.y),0,
		0,0,0,1 };

	//合成用回転行列を宣言し、ZXYの順に合成
	Matrix4 matRot = matRotZ;
	matRot *= matRotX;
	matRot *= matRotY;

	//////平行移動//////
	//X,Y,Z方向の平行移動を設定
	worldTransform_.translation_ = { 0,10,0 };
	//移動量を行列に設定する
	Matrix4 matTrans = 
		{ 1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		worldTransform_.translation_.x,worldTransform_.translation_.y,worldTransform_.translation_.z,1 };


	//単位行列を代入
	worldTransform_.matWorld_ =
	{ 1,0,0,0,
	0,1,0,0,
	0,0,1,0,
	0,0,0,1 };
	//スケーリング行列を掛ける
	worldTransform_.matWorld_ *= matScale;
	//合成用回転行列を掛ける
	worldTransform_.matWorld_ *= matRot;
	//平行移動行列を掛ける
	worldTransform_.matWorld_ *= matTrans;

	//行列の転送
	worldTransform_.TransferMatrix();

	//ビュープロジェクションの初期化
	viewProjection_.Initialize();

	//デバッグカメラの生成
	debugCamera_ = new DebugCamera(400,300);

	//軸方向の表示を有効にする
	AxisIndicator::GetInstance()->SetVisible(true);
	//軸方向表示が参照するビュープロジェクションを指定する（アドレス渡し）
	AxisIndicator::GetInstance()->SetTargetViewProjection(&debugCamera_->GetViewProjection());

	//ライン描画が参照するビュープロジェクションを指定する（アドレス渡し）
	PrimitiveDrawer::GetInstance()->SetViewProjection(&debugCamera_->GetViewProjection());
}

void GameScene::Update() {
	debugCamera_->Update();
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
	model_->Draw(worldTransform_, debugCamera_->GetViewProjection(), textureHandle_);

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
