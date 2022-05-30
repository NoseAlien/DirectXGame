#pragma once
#include "Model.h"
#include "WorldTransform.h"
#include "Input.h"
#include "DebugText.h"
#include "ViewProjection.h"

///<summary>
///自キャラ
///</summary>
class Player
{
public:
	///<summary>
	///初期化
	///</summary>
	void Initialize(Model* model,uint32_t textureHandle);

	///<summary>
	///更新
	///</summary>
	void Update();

	///<summary>
	///描画
	///</summary>
	void Draw(ViewProjection viewProjection);
private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	uint32_t textureHandle_ = 0u;
	Input* input_ = nullptr;
	DebugText* debugText_ = nullptr;
};