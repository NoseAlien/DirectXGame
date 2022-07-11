#pragma once

#include "Audio.h"
#include "DirectXCommon.h"
#include "DebugText.h"
#include "Input.h"
#include "Model.h"
#include "SafeDelete.h"
#include "Sprite.h"
#include "ViewProjection.h"
#include "WorldTransform.h"
#include "DebugCamera.h"
#include "Player.h"

/// <summary>
/// ゲームシーン
/// </summary>
class GameScene {

  public: // メンバ関数
	/// <summary>
	/// コンストクラタ
	/// </summary>
	GameScene();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~GameScene();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 毎フレーム処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	//パーツID
	enum PartID
	{
		kRoot,//大元
		kSpine,//脊椎
		kChest,//胸
		kHead,//頭
		kArmL,//左腕
		kArmR,//右腕
		kHandL,//左手
		kHandR,//右手
		kHip,//尻
		kLegL,//左足
		kLegR,//右足
		kFootL,//左足先
		kFootR,//右足先

		kNumPartID
	};

  private: // メンバ変数
	DirectXCommon* dxCommon_ = nullptr;
	Input* input_ = nullptr;
	Audio* audio_ = nullptr;
	DebugText* debugText_ = nullptr;

	/// <summary>
	/// ゲームシーン用
	/// </summary>
	
	//テクスチャハンドル
	uint32_t textureHandle_ = 0;

	uint32_t cursorTex = 0;

	//3Dモデル
	Model* model_ = nullptr;

	Sprite* sprite_ = nullptr;

	//ビュープロジェクション
	ViewProjection viewProjection_;
	//カメラ上方向の角度
	float viewAngleUp = 0.0f;

	//デバッグカメラ
	DebugCamera* debugCamera_ = nullptr;

	//デバッグカメラ有効
	bool isDebugCameraActive_ = false;

	WorldTransform worldTransform_[100];

	float armAndLegAngle = 0;

	bool rotFlag = false;

	float jumpState = 0;

	WorldTransform* playable_ = nullptr;

	//自キャラ
	Player* player_ = nullptr;
};
