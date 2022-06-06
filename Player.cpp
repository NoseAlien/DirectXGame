#include "Player.h"
#include <cassert>

void Player::Initialize(Model* model, uint32_t textureHandle)
{
	assert(model);

	model_ = model;
	textureHandle_ = textureHandle;

	input_ = Input::GetInstance();
	debugText_ = DebugText::GetInstance();

	worldTransform_.Initialize();
}

void Player::Update()
{
	Vector3 move = { 0,0,0 };

	if (input_->PushKey(DIK_D))
	{
		move.x += 0.1;
	}
	if (input_->PushKey(DIK_A))
	{
		move.x -= 0.1;
	}

	if (input_->PushKey(DIK_W))
	{
		move.y += 0.1;
	}
	if (input_->PushKey(DIK_S))
	{
		move.y -= 0.1;
	}

	worldTransform_.translation_ += move;

	const float kMoveLimitX = 20;
	const float kMoveLimitY = 10;

	worldTransform_.translation_.x = max(worldTransform_.translation_.x, -kMoveLimitX);
	worldTransform_.translation_.x = min(worldTransform_.translation_.x, kMoveLimitX);
	worldTransform_.translation_.y = max(worldTransform_.translation_.y, -kMoveLimitY);
	worldTransform_.translation_.y = min(worldTransform_.translation_.y, kMoveLimitY);
	
	worldTransform_.UpdateMatrix();

	debugText_->SetPos(50, 110);
	debugText_->Printf(
		"PlayerPos:(%f,%f,%f)", worldTransform_.translation_.x, worldTransform_.translation_.y, worldTransform_.translation_.z);
}

void Player::Draw(ViewProjection viewProjection)
{
	model_->Draw(worldTransform_, viewProjection, textureHandle_);
}

WorldTransform Player::GetWorldTransform()
{
	return worldTransform_;
}
