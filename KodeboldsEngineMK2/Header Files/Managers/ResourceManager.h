#pragma once
#include <memory>
#include <vector>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include "DDSTextureLoader.h"
#include <string>
#include "ObjLoader.h"
#include <wrl/client.h>
#include "VBO_DX.h"
#include "ShaderObject_DX.h"
#include "TextureObject_DX.h"
#include "ShaderObject_GL.h"
#include "TextureObject_GL.h"
#include "ECSManager.h"
#include "Sprite.h"
#include <Audio.h>
#include "Sound.h"

class RenderSystem;

class ResourceManager
{
	std::vector< std::pair< std::wstring, TextureObject*>> mTextures{};
	std::vector< std::pair< std::wstring, VBO*>> mGeometries{};
	std::vector< std::pair< std::wstring, ShaderObject*>> mShaders{};
	//std::vector< std::pair< std::wstring, Microsoft::WRL::ComPtr< ID3D11Buffer >>> mInstances{};
	//std::vector< std::pair< string, AUDIOBUFFER>> mSounds;


	//Private constructor for singleton pattern
	ResourceManager();

public:
	~ResourceManager();

	std::vector<std::pair<std::wstring, Sound*>> mSounds;
	std::vector<std::pair<std::wstring, Sprite>> mSprites{};
	//Singleton pattern
	//Deleted copy constructor and assignment operator so no copies of the singleton instance can be made
	ResourceManager(const ResourceManager& pResourceManager) = delete;
	ResourceManager& operator=(ResourceManager const&) = delete;

	const TextureObject* const LoadTexture(const RenderSystem* const pRenderer, const std::wstring& pFilename);
	VBO* const LoadGeometry(const RenderSystem* const pRenderer, const std::wstring& pFilename);
	//AUDIOBUFFER LoadAudio(string filename);
	const ShaderObject* const LoadShader(const RenderSystem* const pRenderer, const std::wstring& pFilename);


	static std::shared_ptr< ResourceManager > Instance();

	DirectX::AUDIO_ENGINE_FLAGS eflags = DirectX::AUDIO_ENGINE_FLAGS::AudioEngine_Default;
	std::unique_ptr<DirectX::AudioEngine> mAudEngine = std::make_unique<DirectX::AudioEngine>(eflags);
	Sound* LoadAudio(std::wstring pFileName);
};

