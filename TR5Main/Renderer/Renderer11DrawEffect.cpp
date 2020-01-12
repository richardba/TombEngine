#include "Renderer11.h"
#include "../Game/footprint.h"

extern BLOOD_STRUCT Blood[MAX_SPARKS_BLOOD];
extern FIRE_SPARKS FireSparks[MAX_SPARKS_FIRE];
extern SMOKE_SPARKS SmokeSparks[MAX_SPARKS_SMOKE];
extern BUBBLE_STRUCT Bubbles[MAX_BUBBLES];
extern DRIP_STRUCT Drips[MAX_DRIPS];
extern SHOCKWAVE_STRUCT ShockWaves[MAX_SHOCKWAVE];
extern FIRE_LIST Fires[MAX_FIRE_LIST];
extern GUNFLASH_STRUCT Gunflashes[MAX_GUNFLASH]; // offset 0xA31D8
extern SPARKS Sparks[MAX_SPARKS];
extern SPLASH_STRUCT Splashes[MAX_SPLASH];
extern RIPPLE_STRUCT Ripples[MAX_RIPPLES];
extern std::deque<FOOTPRINT_STRUCT> footprints;

void Renderer11::AddSprite3D(RendererSprite* sprite, Vector3 vtx1, Vector3 vtx2, Vector3 vtx3, Vector3 vtx4, Vector4 color, float rotation, float scale, float width, float height, BLEND_MODES blendMode)
{
	if (m_nextSprite >= MAX_SPRITES)
		return;

	scale = 1.0f;

	width *= scale;
	height *= scale;

	RendererSpriteToDraw * spr = &m_spritesBuffer[m_nextSprite++];

	spr->Type = RENDERER_SPRITE_TYPE::SPRITE_TYPE_3D;
	spr->Sprite = sprite;
	spr->vtx1 = vtx1;
	spr->vtx2 = vtx2;
	spr->vtx3 = vtx3;
	spr->vtx4 = vtx4;
	spr->color = color;
	spr->Rotation = rotation;
	spr->Scale = scale;
	spr->Width = width;
	spr->Height = height;
	spr->BlendMode = blendMode;

	m_spritesToDraw.push_back(spr);
}

void Renderer11::drawSmokes()
{
	for (int i = 0; i < 32; i++)
	{
		SMOKE_SPARKS* spark = &SmokeSparks[i];

		if (spark->on)
		{
			AddSpriteBillboard(m_sprites[spark->def],
				Vector3(spark->x, spark->y, spark->z),
				Vector4(spark->shade / 255.0f, spark->shade / 255.0f, spark->shade / 255.0f, 1.0f),
				TR_ANGLE_TO_RAD(spark->rotAng), spark->scalar, spark->size * 4.0f, spark->size * 4.0f,
				BLENDMODE_ALPHABLEND);
		}
	}
}

void Renderer11::AddSpriteBillboard(RendererSprite* sprite, Vector3 pos, Vector4 color, float rotation, float scale, float width, float height, BLEND_MODES blendMode)
{
	if (m_nextSprite >= MAX_SPRITES)
		return;

	scale = 1.0f;

	width *= scale;
	height *= scale;

	RendererSpriteToDraw * spr = &m_spritesBuffer[m_nextSprite++];

	spr->Type = RENDERER_SPRITE_TYPE::SPRITE_TYPE_BILLBOARD;
	spr->Sprite = sprite;
	spr->pos = pos;
	spr->color = color;
	spr->Rotation = rotation;
	spr->Scale = scale;
	spr->Width = width;
	spr->Height = height;
	spr->BlendMode = blendMode;

	m_spritesToDraw.push_back(spr);
}

void Renderer11::drawFires()
{
	for (int k = 0; k < MAX_FIRE_LIST; k++)
	{
		FIRE_LIST* fire = &Fires[k];
		if (fire->on)
		{
			for (int i = 0; i < MAX_SPARKS_FIRE; i++)
			{
				FIRE_SPARKS* spark = &FireSparks[i];
				if (spark->on)
					AddSpriteBillboard(m_sprites[spark->def], Vector3(fire->x + spark->x, fire->y + spark->y, fire->z + spark->z), Vector4(spark->r / 255.0f, spark->g / 255.0f, spark->b / 255.0f, 1.0f), TR_ANGLE_TO_RAD(spark->rotAng), spark->scalar, spark->size * 4.0f, spark->size * 4.0f, BLENDMODE_ALPHABLEND);
			}
		}
	}
}

void Renderer11::drawSparks()
{
	for (int i = 0; i < MAX_SPARKS; i++)
	{
		SPARKS* spark = &Sparks[i];
		if (spark->on)
		{
			if (spark->flags & SP_DEF)
			{
				FX_INFO* fx = &Effects[spark->fxObj];

				AddSpriteBillboard(m_sprites[spark->def],
					Vector3(fx->pos.xPos + spark->x, fx->pos.yPos + spark->y, fx->pos.zPos + spark->z),
					Vector4(spark->r / 255.0f, spark->g / 255.0f, spark->b / 255.0f, 1.0f),
					TR_ANGLE_TO_RAD(spark->rotAng), spark->scalar, spark->size * 12.0f, spark->size * 12.0f,
					BLENDMODE_ALPHABLEND);
			}
			else
			{
				Vector3 v = Vector3(spark->xVel, spark->yVel, spark->zVel);
				v.Normalize();
				AddLine3D(Vector3(spark->x, spark->y, spark->z), Vector3(spark->x + v.x * 24.0f, spark->y + v.y * 24.0f, spark->z + v.z * 24.0f), Vector4(spark->r / 255.0f, spark->g / 255.0f, spark->b / 255.0f, 1.0f));
			}
		}
	}
}

void Renderer11::drawSplahes()
{
	constexpr size_t NUM_POINTS = 12;
	for (int i = 0; i < MAX_SPLASH; i++)
	{
		SPLASH_STRUCT& splash = Splashes[i];
		if (splash.isActive)
		{
			constexpr float alpha = 360 / NUM_POINTS;
			byte color = (splash.life >= 32 ? 255 : (byte)((splash.life / 32.0f) * 255));
			if (!splash.isRipple) {
				if (splash.heightSpeed < 0 && splash.height < 1024) {
					float multiplier = splash.height / 1024.0f;
					color = (float)color * multiplier;
				}
			}
			float innerRadius = splash.innerRad;
			float outerRadius = splash.outerRad;
			float xInner;
			float zInner;
			float xOuter;
			float zOuter;
			float x2Inner;
			float z2Inner;
			float x2Outer;
			float z2Outer;
			float yInner = splash.y;
			float yOuter = splash.y - splash.height;
			for (int i = 0; i < NUM_POINTS; i++) {
				xInner = innerRadius * sin(alpha * i * PI / 180);
				zInner = innerRadius * cos(alpha * i * PI / 180);
				xOuter = outerRadius * sin(alpha * i * PI / 180);
				zOuter = outerRadius * cos(alpha * i * PI / 180);
				xInner += splash.x;
				zInner += splash.z;
				xOuter += splash.x;
				zOuter += splash.z;
				int j = (i + 1) % NUM_POINTS;
				x2Inner = innerRadius * sin(alpha * j * PI / 180);
				x2Inner += splash.x;
				z2Inner = innerRadius * cos(alpha * j * PI / 180);
				z2Inner += splash.z;
				x2Outer = outerRadius * sin(alpha * j * PI / 180);
				x2Outer += splash.x;
				z2Outer = outerRadius * cos(alpha * j * PI / 180);
				z2Outer += splash.z;
				AddSprite3D(m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + splash.spriteSequenceStart + (int)splash.animationPhase], Vector3(xOuter, yOuter, zOuter), Vector3(x2Outer, yOuter, z2Outer), Vector3(x2Inner, yInner, z2Inner), Vector3(xInner, yInner, zInner), Vector4(color / 255.0f, color / 255.0f, color / 255.0f, 1.0f), 0, 1, 0, 0, BLENDMODE_ALPHABLEND);
			}
		}
	}
}

void Renderer11::drawBubbles()
{
	for (int i = 0; i < MAX_BUBBLES; i++)
	{
		BUBBLE_STRUCT* bubble = &Bubbles[i];
		if (bubble->size)
			AddSpriteBillboard(m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_BUBBLES], Vector3(bubble->pos.x, bubble->pos.y, bubble->pos.z), Vector4(bubble->shade / 255.0f, bubble->shade / 255.0f, bubble->shade / 255.0f, 1.0f), 0.0f, 1.0f, bubble->size * 0.5f, bubble->size * 0.5f, BLENDMODE_ALPHABLEND);
	}
}

void Renderer11::drawDrips()
{
	for (int i = 0; i < MAX_DRIPS; i++)
	{
		DRIP_STRUCT* drip = &Drips[i];

		if (drip->on)
		{
			AddLine3D(Vector3(drip->x, drip->y, drip->z), Vector3(drip->x, drip->y + 24.0f, drip->z), Vector4(drip->r / 255.0f, drip->g / 255.0f, drip->b / 255.0f, 1.0f));
		}
	}
}

void Renderer11::drawRipples()
{
	for (int i = 0; i < MAX_RIPPLES; i++)
	{
		RIPPLE_STRUCT* ripple = &Ripples[i];

		if (ripple->flags & 1)
		{
			float x1 = ripple->x - ripple->size;
			float z1 = ripple->z - ripple->size;
			float x2 = ripple->x + ripple->size;
			float z2 = ripple->z + ripple->size;
			float y = ripple->y;

			byte color = (ripple->init ? ripple->init << 1 : ripple->life << 1);

			AddSprite3D(m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_RIPPLES], Vector3(x1, y, z2), Vector3(x2, y, z2), Vector3(x2, y, z1), Vector3(x1, y, z1), Vector4(color / 255.0f, color / 255.0f, color / 255.0f, 1.0f), 0.0f, 1.0f, ripple->size, ripple->size, BLENDMODE_ALPHABLEND);
		}
	}
}

void Renderer11::drawShockwaves()
{
	for (int i = 0; i < MAX_SHOCKWAVE; i++)
	{
		SHOCKWAVE_STRUCT* shockwave = &ShockWaves[i];

		if (shockwave->life)
		{
			byte color = shockwave->life * 8;

			// Inner circle
			float angle = PI / 32.0f;
			float c = cos(angle);
			float s = sin(angle);
			float x1 = shockwave->x + (shockwave->innerRad * c);
			float z1 = shockwave->z + (shockwave->innerRad * s);
			float x4 = shockwave->x + (shockwave->outerRad * c);
			float z4 = shockwave->z + (shockwave->outerRad * s);
			angle -= PI / 8.0f;

			for (int j = 0; j < 16; j++)
			{
				c = cos(angle);
				s = sin(angle);
				float x2 = shockwave->x + (shockwave->innerRad * c);
				float z2 = shockwave->z + (shockwave->innerRad * s);
				float x3 = shockwave->x + (shockwave->outerRad * c);
				float z3 = shockwave->z + (shockwave->outerRad * s);
				angle -= PI / 8.0f;

				AddSprite3D(m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_SPLASH],
					Vector3(x1, shockwave->y, z1),
					Vector3(x2, shockwave->y, z2),
					Vector3(x3, shockwave->y, z3),
					Vector3(x4, shockwave->y, z4),
					Vector4(color / 255.0f, color / 255.0f, color / 255.0f, 1.0f), 0, 1, 0, 0, BLENDMODE_ALPHABLEND);

				x1 = x2;
				z1 = z2;
				x4 = x3;
				z4 = z3;
			}
		}
	}
}

void Renderer11::drawBlood()
{
	for (int i = 0; i < 32; i++)
	{
		BLOOD_STRUCT* blood = &Blood[i];

		if (blood->on)
		{
			AddSpriteBillboard(m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + 15],
				Vector3(blood->x, blood->y, blood->z),
				Vector4(blood->shade / 255.0f, blood->shade * 0, blood->shade * 0, 1.0f),
				TR_ANGLE_TO_RAD(blood->rotAng), 1.0f, blood->size * 8.0f, blood->size * 8.0f,
				BLENDMODE_ALPHABLEND);
		}
	}
}

bool Renderer11::drawGunFlashes()
{
	if (!Lara.rightArm.flash_gun && !Lara.leftArm.flash_gun)
		return true;

	Matrix world;
	Matrix translation;
	Matrix rotation;

	RendererObject* laraObj = m_moveableObjects[ID_LARA];
	RendererObject* laraSkin = m_moveableObjects[ID_LARA_SKIN];

	OBJECT_INFO* obj = &Objects[0];
	RendererRoom& const room = m_rooms[LaraItem->roomNumber];
	RendererItem* item = &m_items[Lara.itemNumber];

	m_stItem.AmbientLight = room.AmbientLight;
	memcpy(m_stItem.BonesMatrices, &Matrix::Identity, sizeof(Matrix));

	m_stLights.NumLights = item->Lights.Size();
	for (int j = 0; j < item->Lights.Size(); j++)
		memcpy(&m_stLights.Lights[j], item->Lights[j], sizeof(ShaderLight));
	updateConstantBuffer(m_cbLights, &m_stLights, sizeof(CLightBuffer));
	m_context->PSSetConstantBuffers(2, 1, &m_cbLights);

	m_stMisc.AlphaTest = true;
	updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
	m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);

	short length = 0;
	short zOffset = 0;
	short rotationX = 0;

	m_context->OMSetBlendState(m_states->Additive(), NULL, 0xFFFFFFFF);
	m_context->OMSetDepthStencilState(m_states->DepthNone(), 0);

	if (Lara.weaponItem != WEAPON_FLARE && Lara.weaponItem != WEAPON_SHOTGUN && Lara.weaponItem != WEAPON_CROSSBOW)
	{
		switch (Lara.weaponItem)
		{
		case WEAPON_REVOLVER:
			length = 192;
			zOffset = 68;
			rotationX = -14560;
			break;
		case WEAPON_UZI:
			length = 190;
			zOffset = 50;
			break;
		case WEAPON_HK:
			length = 300;
			zOffset = 92;
			rotationX = -14560;
			break;
		default:
		case WEAPON_PISTOLS:
			length = 180;
			zOffset = 40;
			rotationX = -16830;
			break;
		}

		OBJECT_INFO* flashObj = &Objects[ID_GUN_FLASH];
		RendererObject* flashMoveable = m_moveableObjects[ID_GUN_FLASH];
		RendererMesh* flashMesh = flashMoveable->ObjectMeshes[0];

		for (int b = 0; b < NUM_BUCKETS; b++)
		{
			RendererBucket* flashBucket = &flashMesh->Buckets[b];

			if (flashBucket->NumVertices != 0)
			{
				Matrix offset = Matrix::CreateTranslation(0, length, zOffset);
				Matrix rotation2 = Matrix::CreateRotationX(TR_ANGLE_TO_RAD(rotationX));

				if (Lara.leftArm.flash_gun)
				{
					world = laraObj->AnimationTransforms[LM_LHAND].Transpose() * m_LaraWorldMatrix;
					world = offset * world;
					world = rotation2 * world;

					m_stItem.World = world.Transpose();
					updateConstantBuffer(m_cbItem, &m_stItem, sizeof(CItemBuffer));
					m_context->VSSetConstantBuffers(1, 1, &m_cbItem);

					m_context->DrawIndexed(flashBucket->NumIndices, flashBucket->StartIndex, 0);
					m_numDrawCalls++;
				}

				if (Lara.rightArm.flash_gun)
				{
					world = laraObj->AnimationTransforms[LM_RHAND].Transpose() * m_LaraWorldMatrix;
					world = offset * world;
					world = rotation2 * world;

					m_stItem.World = world.Transpose();
					updateConstantBuffer(m_cbItem, &m_stItem, sizeof(CItemBuffer));
					m_context->VSSetConstantBuffers(1, 1, &m_cbItem);

					m_context->DrawIndexed(flashBucket->NumIndices, flashBucket->StartIndex, 0);
					m_numDrawCalls++;
				}
			}
		}
	}

	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
	m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

	return true;
}

void Renderer11::drawFootprints()
{
	for (auto i = footprints.begin(); i != footprints.end(); i++) {
		FOOTPRINT_STRUCT& footprint = *i;
		if (footprint.active) {
			float y = footprint.pos.yPos;
			Matrix rot = Matrix::CreateRotationY(TR_ANGLE_TO_RAD(footprint.pos.yRot) + PI);
			Vector3 p1 = Vector3(-64, 0, -64);
			Vector3 p2 = Vector3(64, 0, -64);
			Vector3 p3 = Vector3(64, 0, 64);
			Vector3 p4 = Vector3(-64, 0, 64);
			p1 = XMVector3Transform(p1, rot);
			p2 = XMVector3Transform(p2, rot);
			p3 = XMVector3Transform(p3, rot);
			p4 = XMVector3Transform(p4, rot);
			p1 += Vector3(footprint.pos.xPos, footprint.pos.yPos, footprint.pos.zPos);
			p2 += Vector3(footprint.pos.xPos, footprint.pos.yPos, footprint.pos.zPos);
			p3 += Vector3(footprint.pos.xPos, footprint.pos.yPos, footprint.pos.zPos);
			p4 += Vector3(footprint.pos.xPos, footprint.pos.yPos, footprint.pos.zPos);
			if (m_sprites[Objects[ID_MISC_SPRITES].meshIndex + 1] != nullptr)
				AddSprite3D(m_sprites[Objects[ID_MISC_SPRITES].meshIndex + 1], p1, p2, p3, p4, Vector4(footprint.opacity / 255.0f, footprint.opacity / 255.0f, footprint.opacity / 255.0f, footprint.opacity / 255.0f),
					0, 1, 1, 1, BLENDMODE_SUBTRACTIVE);
		}
	}
}

void Renderer11::drawUnderwaterDust()
{
	if (m_firstUnderwaterDustParticles)
	{
		for (int i = 0; i < NUM_UNDERWATER_DUST_PARTICLES; i++)
			m_underwaterDustParticles[i].Reset = true;
	}

	for (int i = 0; i < NUM_UNDERWATER_DUST_PARTICLES; i++)
	{
		RendererUnderwaterDustParticle* dust = &m_underwaterDustParticles[i];

		if (dust->Reset)
		{
			dust->X = LaraItem->pos.xPos + rand() % UNDERWATER_DUST_PARTICLES_RADIUS - UNDERWATER_DUST_PARTICLES_RADIUS / 2.0f;
			dust->Y = LaraItem->pos.yPos + rand() % UNDERWATER_DUST_PARTICLES_RADIUS - UNDERWATER_DUST_PARTICLES_RADIUS / 2.0f;
			dust->Z = LaraItem->pos.zPos + rand() % UNDERWATER_DUST_PARTICLES_RADIUS - UNDERWATER_DUST_PARTICLES_RADIUS / 2.0f;

			// Check if water room
			short roomNumber = Camera.pos.roomNumber;
			FLOOR_INFO* floor = GetFloor(dust->X, dust->Y, dust->Z, &roomNumber);
			if (!isRoomUnderwater(roomNumber))
				continue;

			if (!isInRoom(dust->X, dust->Y, dust->Z, roomNumber))
			{
				dust->Reset = true;
				continue;
			}

			dust->Life = 0;
			dust->Reset = false;
		}

		dust->Life++;
		byte color = (dust->Life > 16 ? 32 - dust->Life : dust->Life) * 4;

		AddSpriteBillboard(m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_UNDERWATERDUST], Vector3(dust->X, dust->Y, dust->Z), Vector4(color / 255.0f, color / 255.0f, color / 255.0f, 1.0f), 0.0f, 1.0f, 12, 12, BLENDMODE_ALPHABLEND);

		if (dust->Life >= 32)
			dust->Reset = true;
	}

	m_firstUnderwaterDustParticles = false;

	return;
}

bool Renderer11::drawSprites()
{
	m_context->RSSetState(m_states->CullNone());
	m_context->OMSetDepthStencilState(m_states->DepthRead(), 0);

	m_context->VSSetShader(m_vsSprites, NULL, 0);
	m_context->PSSetShader(m_psSprites, NULL, 0);

	m_stCameraMatrices.View = View.Transpose();
	m_stCameraMatrices.Projection = Projection.Transpose();
	updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);

	m_stMisc.AlphaTest = true;
	updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
	m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);

	m_context->PSSetShaderResources(0, 1, &m_textureAtlas->ShaderResourceView);
	ID3D11SamplerState* sampler = m_states->AnisotropicClamp();
	m_context->PSSetSamplers(0, 1, &sampler);

	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout);

	for (int b = 0; b < 4; b++)
	{
		BLEND_MODES currentBlendMode = (BLEND_MODES)b;

		int numSpritesToDraw = m_spritesToDraw.size();
		int lastSprite = 0;

		m_primitiveBatch->Begin();

		for (int i = 0; i < numSpritesToDraw; i++)
		{
			RendererSpriteToDraw* spr = m_spritesToDraw[i];

			if (spr->BlendMode != currentBlendMode)
				continue;
			switch (currentBlendMode) {
			case BLENDMODE_ALPHABLEND:
				m_context->OMSetBlendState(m_states->Additive(), NULL, 0xFFFFFFFF);

				break;
			case BLENDMODE_ALPHATEST:
				m_context->OMSetBlendState(m_states->Additive(), NULL, 0xFFFFFFFF);

				break;
			case BLENDMODE_OPAQUE:
				m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
				break;
			case BLENDMODE_SUBTRACTIVE:
				m_context->OMSetBlendState(m_subtractiveBlendState, NULL, 0xFFFFFFFF);

				break;
			}

			if (spr->Type == RENDERER_SPRITE_TYPE::SPRITE_TYPE_BILLBOARD)
			{
				float halfWidth = spr->Width / 2.0f;
				float halfHeight = spr->Height / 2.0f;

				Matrix billboardMatrix;
				createBillboardMatrix(&billboardMatrix, &spr->pos, &Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z), spr->Rotation);

				Vector3 p0 = Vector3(-halfWidth, -halfHeight, 0);
				Vector3 p1 = Vector3(halfWidth, -halfHeight, 0);
				Vector3 p2 = Vector3(halfWidth, halfHeight, 0);
				Vector3 p3 = Vector3(-halfWidth, halfHeight, 0);

				Vector3 p0t = Vector3::Transform(p0, billboardMatrix);
				Vector3 p1t = Vector3::Transform(p1, billboardMatrix);
				Vector3 p2t = Vector3::Transform(p2, billboardMatrix);
				Vector3 p3t = Vector3::Transform(p3, billboardMatrix);

				RendererVertex v0;
				v0.Position.x = p0t.x;
				v0.Position.y = p0t.y;
				v0.Position.z = p0t.z;
				v0.UV.x = spr->Sprite->UV[0].x;
				v0.UV.y = spr->Sprite->UV[0].y;
				v0.Color = spr->color;

				RendererVertex v1;
				v1.Position.x = p1t.x;
				v1.Position.y = p1t.y;
				v1.Position.z = p1t.z;
				v1.UV.x = spr->Sprite->UV[1].x;
				v1.UV.y = spr->Sprite->UV[1].y;
				v1.Color = spr->color;

				RendererVertex v2;
				v2.Position.x = p2t.x;
				v2.Position.y = p2t.y;
				v2.Position.z = p2t.z;
				v2.UV.x = spr->Sprite->UV[2].x;
				v2.UV.y = spr->Sprite->UV[2].y;
				v2.Color = spr->color;

				RendererVertex v3;
				v3.Position.x = p3t.x;
				v3.Position.y = p3t.y;
				v3.Position.z = p3t.z;
				v3.UV.x = spr->Sprite->UV[3].x;
				v3.UV.y = spr->Sprite->UV[3].y;
				v3.Color = spr->color;

				m_primitiveBatch->DrawQuad(v0, v1, v2, v3);
			}
			else if (spr->Type == RENDERER_SPRITE_TYPE::SPRITE_TYPE_3D)
			{
				Vector3 p0t = spr->vtx1;
				Vector3 p1t = spr->vtx2;
				Vector3 p2t = spr->vtx3;
				Vector3 p3t = spr->vtx4;

				RendererVertex v0;
				v0.Position.x = p0t.x;
				v0.Position.y = p0t.y;
				v0.Position.z = p0t.z;
				v0.UV.x = spr->Sprite->UV[0].x;
				v0.UV.y = spr->Sprite->UV[0].y;
				v0.Color = spr->color;

				RendererVertex v1;
				v1.Position.x = p1t.x;
				v1.Position.y = p1t.y;
				v1.Position.z = p1t.z;
				v1.UV.x = spr->Sprite->UV[1].x;
				v1.UV.y = spr->Sprite->UV[1].y;
				v1.Color = spr->color;

				RendererVertex v2;
				v2.Position.x = p2t.x;
				v2.Position.y = p2t.y;
				v2.Position.z = p2t.z;
				v2.UV.x = spr->Sprite->UV[2].x;
				v2.UV.y = spr->Sprite->UV[2].y;
				v2.Color = spr->color;

				RendererVertex v3;
				v3.Position.x = p3t.x;
				v3.Position.y = p3t.y;
				v3.Position.z = p3t.z;
				v3.UV.x = spr->Sprite->UV[3].x;
				v3.UV.y = spr->Sprite->UV[3].y;
				v3.Color = spr->color;

				m_primitiveBatch->DrawQuad(v0, v1, v2, v3);
			}
		}

		m_primitiveBatch->End();
	}

	m_context->RSSetState(m_states->CullCounterClockwise());
	m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

	return true;
}

bool Renderer11::drawEffect(RendererEffect* effect, bool transparent)
{
	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;

	int firstBucket = (transparent ? 2 : 0);
	int lastBucket = (transparent ? 4 : 2);

	RendererRoom & const room = m_rooms[effect->Effect->roomNumber];
	RendererObject * moveableObj = m_moveableObjects[effect->Effect->objectNumber];

	m_stItem.World = effect->World.Transpose();
	m_stItem.Position = Vector4(effect->Effect->pos.xPos, effect->Effect->pos.yPos, effect->Effect->pos.zPos, 1.0f);
	m_stItem.AmbientLight = room.AmbientLight;
	Matrix matrices[1] = { Matrix::Identity };
	memcpy(m_stItem.BonesMatrices, matrices, sizeof(Matrix));
	updateConstantBuffer(m_cbItem, &m_stItem, sizeof(CItemBuffer));
	m_context->VSSetConstantBuffers(1, 1, &m_cbItem);

	m_stLights.NumLights = effect->Lights.Size();
	for (int j = 0; j < effect->Lights.Size(); j++)
		memcpy(&m_stLights.Lights[j], effect->Lights[j], sizeof(ShaderLight));
	updateConstantBuffer(m_cbLights, &m_stLights, sizeof(CLightBuffer));
	m_context->PSSetConstantBuffers(2, 1, &m_cbLights);

	m_stMisc.AlphaTest = !transparent;
	updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
	m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);

	RendererMesh * mesh = effect->Mesh;

	for (int j = firstBucket; j < lastBucket; j++)
	{
		RendererBucket* bucket = &mesh->Buckets[j];

		if (bucket->Vertices.size() == 0)
			continue;

		if (j == RENDERER_BUCKET_SOLID_DS || j == RENDERER_BUCKET_TRANSPARENT_DS)
			m_context->RSSetState(m_states->CullNone());
		else
			m_context->RSSetState(m_states->CullCounterClockwise());

		// Draw vertices
		m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
		m_numDrawCalls++;
	}

	return true;
}

bool Renderer11::drawEffects(bool transparent)
{
	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;

	int firstBucket = (transparent ? 2 : 0);
	int lastBucket = (transparent ? 4 : 2);

	m_context->IASetVertexBuffers(0, 1, &m_moveablesVertexBuffer->Buffer, &stride, &offset);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout);
	m_context->IASetIndexBuffer(m_moveablesIndexBuffer->Buffer, DXGI_FORMAT_R32_UINT, 0);

	for (int i = 0; i < m_effectsToDraw.size(); i++)
	{
		RendererEffect* effect = m_effectsToDraw[i];
		RendererRoom& const room = m_rooms[effect->Effect->roomNumber];
		OBJECT_INFO* obj = &Objects[effect->Effect->objectNumber];

		drawEffect(effect, transparent);

		if (obj->nmeshes > 0)
		{

		}
		else
		{

		}
	}

	return true;
}

bool Renderer11::drawWaterfalls()
{
	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;

	// Draw waterfalls
	m_context->RSSetState(m_states->CullCounterClockwise());

	for (int i = 0; i < m_itemsToDraw.size(); i++)
	{
		RendererItem* item = m_itemsToDraw[i];
		RendererRoom& const room = m_rooms[item->Item->roomNumber];
		RendererObject* moveableObj = m_moveableObjects[item->Item->objectNumber];

		short objectNumber = item->Item->objectNumber;
		if (objectNumber >= ID_WATERFALL1 && objectNumber <= ID_WATERFALLSS2)
		{
			RendererRoom& const room = m_rooms[item->Item->roomNumber];
			RendererObject* moveableObj = m_moveableObjects[item->Item->objectNumber];

			m_stItem.World = item->World.Transpose();
			m_stItem.Position = Vector4(item->Item->pos.xPos, item->Item->pos.yPos, item->Item->pos.zPos, 1.0f);
			m_stItem.AmbientLight = room.AmbientLight; //Vector4::One * 0.1f; // room->AmbientLight;
			memcpy(m_stItem.BonesMatrices, item->AnimationTransforms, sizeof(Matrix) * 32);
			updateConstantBuffer(m_cbItem, &m_stItem, sizeof(CItemBuffer));
			m_context->VSSetConstantBuffers(1, 1, &m_cbItem);

			m_stLights.NumLights = item->Lights.Size();
			for (int j = 0; j < item->Lights.Size(); j++)
				memcpy(&m_stLights.Lights[j], item->Lights[j], sizeof(ShaderLight));
			updateConstantBuffer(m_cbLights, &m_stLights, sizeof(CLightBuffer));
			m_context->PSSetConstantBuffers(2, 1, &m_cbLights);

			m_stMisc.AlphaTest = false;
			updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
			m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);

			m_primitiveBatch->Begin();

			for (int k = 0; k < moveableObj->ObjectMeshes.size(); k++)
			{
				RendererMesh* mesh = moveableObj->ObjectMeshes[k];

				for (int b = 0; b < NUM_BUCKETS; b++)
				{
					RendererBucket* bucket = &mesh->Buckets[b];

					if (bucket->Vertices.size() == 0)
						continue;

					for (int p = 0; p < bucket->Polygons.size(); p++)
					{
						RendererPolygon* poly = &bucket->Polygons[p];

						OBJECT_TEXTURE* texture = &ObjectTextures[poly->TextureId];
						int tile = texture->tileAndFlag & 0x7FFF;

						if (poly->Shape == SHAPE_RECTANGLE)
						{
							bucket->Vertices[poly->Indices[0]].UV.y = (texture->vertices[0].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;
							bucket->Vertices[poly->Indices[1]].UV.y = (texture->vertices[1].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;
							bucket->Vertices[poly->Indices[2]].UV.y = (texture->vertices[2].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;
							bucket->Vertices[poly->Indices[3]].UV.y = (texture->vertices[3].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;

							m_primitiveBatch->DrawQuad(bucket->Vertices[poly->Indices[0]],
								bucket->Vertices[poly->Indices[1]],
								bucket->Vertices[poly->Indices[2]],
								bucket->Vertices[poly->Indices[3]]);
						}
						else
						{
							bucket->Vertices[poly->Indices[0]].UV.y = (texture->vertices[0].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;
							bucket->Vertices[poly->Indices[1]].UV.y = (texture->vertices[1].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;
							bucket->Vertices[poly->Indices[2]].UV.y = (texture->vertices[2].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;

							m_primitiveBatch->DrawTriangle(bucket->Vertices[poly->Indices[0]],
								bucket->Vertices[poly->Indices[1]],
								bucket->Vertices[poly->Indices[2]]);
						}
					}
				}
			}

			m_primitiveBatch->End();
		}
		else
		{
			continue;
		}
	}

	return true;
}

bool Renderer11::drawDebris(bool transparent)
{
	UINT cPasses = 1;

	// First collect debrises
	vector<RendererVertex> vertices;

	for (int i = 0; i < NUM_DEBRIS; i++)
	{
		DEBRIS_STRUCT* debris = &Debris[i];

		if (debris->on)
		{
			Matrix translation = Matrix::CreateTranslation(debris->x, debris->y, debris->z);
			Matrix rotation = Matrix::CreateFromYawPitchRoll(TR_ANGLE_TO_RAD(debris->yRot), TR_ANGLE_TO_RAD(debris->xRot), 0);
			Matrix world = rotation * translation;

			OBJECT_TEXTURE* texture = &ObjectTextures[(int)(debris->textInfo) & 0x7FFF];
			int tile = texture->tileAndFlag & 0x7FFF;

			// Draw only debris of the current bucket
			if (texture->attribute == 0 && transparent ||
				texture->attribute == 1 && transparent ||
				texture->attribute == 2 && !transparent)
				continue;

			RendererVertex vertex;

			// Prepare the triangle
			Vector3 p = Vector3(debris->xyzOffsets1[0], debris->xyzOffsets1[1], debris->xyzOffsets1[2]);
			p = Vector3::Transform(p, world);
			vertex.Position.x = p.x;
			vertex.Position.y = p.y;
			vertex.Position.z = p.z;
			vertex.UV.x = (texture->vertices[0].x * 256.0f + 0.5f + GET_ATLAS_PAGE_X(tile)) / (float)TEXTURE_ATLAS_SIZE;
			vertex.UV.y = (texture->vertices[0].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;
			vertex.Color.x = debris->pad[2] / 255.0f;
			vertex.Color.y = debris->pad[3] / 255.0f;
			vertex.Color.z = debris->pad[4] / 255.0f;
			vertices.push_back(vertex);

			p = Vector3(debris->xyzOffsets2[0], debris->xyzOffsets2[1], debris->xyzOffsets2[2]);
			p = Vector3::Transform(p, world);
			vertex.Position.x = p.x;
			vertex.Position.y = p.y;
			vertex.Position.z = p.z;
			vertex.UV.x = (texture->vertices[1].x * 256.0f + 0.5f + GET_ATLAS_PAGE_X(tile)) / (float)TEXTURE_ATLAS_SIZE;
			vertex.UV.y = (texture->vertices[1].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;
			vertex.Color.x = debris->pad[6] / 255.0f;
			vertex.Color.y = debris->pad[7] / 255.0f;
			vertex.Color.z = debris->pad[8] / 255.0f;
			vertices.push_back(vertex);

			p = Vector3(debris->xyzOffsets3[0], debris->xyzOffsets3[1], debris->xyzOffsets3[2]);
			p = Vector3::Transform(p, world);
			vertex.Position.x = p.x;
			vertex.Position.y = p.y;
			vertex.Position.z = p.z;
			vertex.UV.x = (texture->vertices[2].x * 256.0f + 0.5f + GET_ATLAS_PAGE_X(tile)) / (float)TEXTURE_ATLAS_SIZE;
			vertex.UV.y = (texture->vertices[2].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;
			vertex.Color.x = debris->pad[10] / 255.0f;
			vertex.Color.y = debris->pad[11] / 255.0f;
			vertex.Color.z = debris->pad[12] / 255.0f;
			vertices.push_back(vertex);
		}
	}

	// Check if no debris have to be drawn
	if (vertices.size() == 0)
		return true;

	m_primitiveBatch->Begin();

	// Set shaders
	m_context->VSSetShader(m_vsStatics, NULL, 0);
	m_context->PSSetShader(m_psStatics, NULL, 0);

	// Set texture
	m_context->PSSetShaderResources(0, 1, &m_textureAtlas->ShaderResourceView);
	ID3D11SamplerState * sampler = m_states->AnisotropicClamp();
	m_context->PSSetSamplers(0, 1, &sampler);

	// Set camera matrices
	m_stCameraMatrices.View = View.Transpose();
	m_stCameraMatrices.Projection = Projection.Transpose();
	updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);

	m_stMisc.AlphaTest = !transparent;
	updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
	m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);

	m_stStatic.World = Matrix::Identity;
	m_stStatic.Color = Vector4::One;
	updateConstantBuffer(m_cbStatic, &m_stStatic, sizeof(CStaticBuffer));
	m_context->VSSetConstantBuffers(1, 1, &m_cbStatic);

	// Draw vertices
	m_primitiveBatch->Draw(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST, vertices.data(), vertices.size());
	m_numDrawCalls++;

	m_primitiveBatch->End();

	return true;
}