#pragma once
#include "Model.h"
#include "WorldTransform.h"
#include "Input.h"
#include "DebugText.h"
#include "ViewProjection.h"

///<summary>
///���L����
///</summary>
class Player
{
public:
	///<summary>
	///������
	///</summary>
	void Initialize(Model* model,uint32_t textureHandle);

	///<summary>
	///�X�V
	///</summary>
	void Update();

	///<summary>
	///�`��
	///</summary>
	void Draw(ViewProjection viewProjection);

	WorldTransform GetWorldTransform();
private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	uint32_t textureHandle_ = 0u;
	Input* input_ = nullptr;
	DebugText* debugText_ = nullptr;
};