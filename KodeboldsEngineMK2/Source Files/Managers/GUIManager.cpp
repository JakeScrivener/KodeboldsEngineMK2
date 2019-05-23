#include "GUIManager.h"

/// <summary>
/// Default constructor
/// </summary>
GUIManager::GUIManager()
{
}

/// <summary>
/// Default destructor
/// </summary>
GUIManager::~GUIManager()
{
}

/// <summary>
/// Creates a singleton instance of AntTweak Manager if one hasn't been created before
/// Returns pointer to the instance of AntTweak Manager
/// </summary>
/// <returns>Shared pointer to the AntTweak Manager instance</returns>
std::shared_ptr<GUIManager> GUIManager::Instance()
{
	static std::shared_ptr<GUIManager> instance{ new GUIManager };
	return instance;
}

/// <summary>
/// Initialises anttweak GUI
/// </summary>
/// <param name="graphicsAPI">Graphics API used in application</param>
/// <param name="pDevice">Pointer to graphics device</param>
/// <param name="width">Width of window</param>
/// <param name="height">Height of window</param>
void GUIManager::Init(const TwGraphAPI& pGraphicsAPI, void* const pDevice, const int pWidth, const int pHeight) const
{
	TwInit(pGraphicsAPI, pDevice);
	TwWindowSize(pWidth, pHeight);
}

/// <summary>
/// Adds a bar to the anttweak GUI
/// </summary>
/// <param name="barName">Name of bar to be added</param>
void GUIManager::AddBar(const std::string& pBarName)
{
	TwBar* newBar = TwNewBar(pBarName.c_str());
	mBars.emplace_back(std::make_pair(pBarName, newBar));
}

/// <summary>
/// Adds a variable to the anttweak GUI
/// </summary>
/// <param name="barName">Name of the bar to hold the variable</param>
/// <param name="variableName">Name of the variable to be added</param>
/// <param name="variableType">Type of the variable to be added</param>
/// <param name="variable">Pointer to the variable to be added</param>
/// <param name="behaviourDefinition">Anttweak behaviour definition string</param>
void GUIManager::AddVariable(const std::string& pBarName, const std::string& pVariableName, const TwType& pVariableType, const void* const pVariable, const std::string& pBehaviourDefinition)
{
	auto it = std::find_if(mBars.begin(), mBars.end(), [&](const std::pair<std::string, TwBar*> bar) { return bar.first == pBarName; });
	TwAddVarRO(it->second, pVariableName.c_str(), pVariableType, const_cast<void*>(pVariable), pBehaviourDefinition.c_str());
}

/// <summary>
/// Deletes a bar from the anttweak GUI
/// </summary>
/// <param name="barName">Name of bar to be deleted</param>
void GUIManager::DeleteBar(const std::string& pBarName)
{
	auto it = std::find_if(mBars.begin(), mBars.end(), [&](const std::pair<std::string, TwBar*> bar) { return bar.first == pBarName; });
	TwDeleteBar(it->second);
	mBars.erase(it);
}

/// <summary>
/// Refreshes and draws the anttweak GUI
/// </summary>
void GUIManager::Draw()
{
	for (auto& bar : mBars)
	{
		TwRefreshBar(bar.second);
	}
	TwDraw();
}

/// <summary>
/// Deletes all bars and de-allocates all memory assigned to anttweak
/// </summary>
void GUIManager::Cleanup() const
{
	TwDeleteAllBars();
	TwTerminate();
}

void GUIManager::InititialiseGUI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const int pWidth, const int pHeight)
{
	mDevice = pDevice;
	mDeviceWidth = pWidth;
	mDeviceHeight = pHeight;
	mContext = pContext;

	mSpriteBatch = std::make_unique<DirectX::SpriteBatch>(pContext);
	mStates = std::make_unique<DirectX::CommonStates>(pDevice);
}

void GUIManager::Render()
{

	mSpriteBatch->Begin(DirectX::SpriteSortMode_Deferred, mStates->NonPremultiplied(), nullptr, nullptr, nullptr, nullptr);

	mResourceManager->mSprites.at(0).second.mPosition.x += 1;

	for (int i = 0; i < mResourceManager->mSprites.size(); i++)
	{
		auto sprite = mResourceManager->mSprites[i].second;
		mSpriteBatch->Draw(sprite.mTexture.Get(), sprite.mPosition, nullptr, DirectX::Colors::White, sprite.mRotation, sprite.mOrigin, sprite.mScale);
	}

	mSpriteBatch->End();
}
void GUIManager::RenderText()
{
	mSpriteBatch->Begin(DirectX::SpriteSortMode_Deferred, mStates->NonPremultiplied(), nullptr, nullptr, nullptr, nullptr);
	for (int i = 0; i < mTexts.size(); i++)
	{
		DirectX::XMVECTOR origin = DirectX::XMVectorSet(mTexts[i].mOrigin.x, mTexts[i].mOrigin.y, 0, 0);// mFonts[0]->MeasureString(pText);
		DirectX::XMVECTOR position = DirectX::XMVectorSet(mTexts[i].mPosition.x, mTexts[i].mPosition.y, 0, 0);
		DirectX::XMVECTOR colour = DirectX::XMVectorSet(mTexts[i].mColour.x, mTexts[i].mColour.y, mTexts[i].mColour.z, mTexts[i].mColour.w);

		mFonts[i]->DrawString(mSpriteBatch.get(), mTexts[i].mText, position, colour, mTexts[i].mRotation, origin, 1.0f);
	}
	mSpriteBatch->End();


}

void GUIManager::LoadSprite(const wchar_t* pFileName, KodeboldsMath::Vector2 pOrigin, KodeboldsMath::Vector2 pPosition, KodeboldsMath::Vector2 pPadding, float pRotation, float pScale)
{
	Sprite sprite;
	sprite.mOrigin = DirectX::XMFLOAT2(pOrigin.X, pOrigin.Y);
	sprite.mPosition = DirectX::XMFLOAT2(pPosition.X + pPadding.X, pPosition.Y + pPadding.Y);
	sprite.mRotation = pRotation;
	sprite.mScale = pScale;

	mResourceManager->mSprites.emplace_back(std::make_pair(pFileName, sprite));

	auto imageTest = DirectX::CreateWICTextureFromFile(mDevice.Get(), mContext.Get(), pFileName, nullptr, mResourceManager->mSprites.back().second.mTexture.GetAddressOf());

	if (imageTest == 0x80070002) // file not found
	{
		int i = 0;
	}

	ID3D11Texture2D* pTextureInterface = 0;
	ID3D11Resource* res;
	mResourceManager->mSprites.back().second.mTexture.Get()->GetResource(&res);
	res->QueryInterface<ID3D11Texture2D>(&pTextureInterface);
	D3D11_TEXTURE2D_DESC desc;
	pTextureInterface->GetDesc(&desc);

	mResourceManager->mSprites.back().second.mWidth = desc.Width;
	mResourceManager->mSprites.back().second.mHeight = desc.Height;
}
void GUIManager::LoadSprite(const wchar_t* pFileName, KodeboldsMath::Vector2 pOrigin, SpritePosition pPosition, KodeboldsMath::Vector2 pPadding, float pRotation, float pScale)
{
	KodeboldsMath::Vector2 position;

	switch (pPosition)
	{
	case SpritePosition::CENTRE_TOP:
		position.X = (mDeviceWidth / 2.0f) + pPadding.X;
		position.Y = 0 + pPadding.Y;
		break;
	case SpritePosition::CENTRE_MIDDLE:
		position.X = (mDeviceWidth / 2.0f) + pPadding.X;
		position.Y = (mDeviceHeight / 2.0f) + pPadding.Y;
		break;
	case SpritePosition::CENTRE_BOTTOM:
		position.X = (mDeviceWidth / 2.0f) + pPadding.X;
		position.Y = mDeviceHeight + pPadding.Y;
		break;
	}

	LoadSprite(pFileName, pOrigin, position, pPadding, pRotation, pScale);
}
void GUIManager::LoadSprite(const wchar_t* pFileName, SpriteOrigin pOrigin, KodeboldsMath::Vector2 pPosition, KodeboldsMath::Vector2 pPadding, float pRotation, float pScale)
{
	Sprite sprite;
	sprite.mOrigin = DirectX::XMFLOAT2(0, 0);
	sprite.mPosition = DirectX::XMFLOAT2(pPosition.X + pPadding.X, pPosition.Y + pPadding.Y);
	sprite.mRotation = pRotation;
	sprite.mScale = pScale;

	mResourceManager->mSprites.emplace_back(std::make_pair(pFileName, sprite));

	auto imageTest = DirectX::CreateWICTextureFromFile(mDevice.Get(), mContext.Get(), pFileName, nullptr, mResourceManager->mSprites.back().second.mTexture.GetAddressOf());

	if (imageTest == 0x80070002) // file not found
	{
		int i = 0;
	}

	ID3D11Texture2D* pTextureInterface = 0;
	ID3D11Resource* res;
	mResourceManager->mSprites.back().second.mTexture.Get()->GetResource(&res);
	res->QueryInterface<ID3D11Texture2D>(&pTextureInterface);
	D3D11_TEXTURE2D_DESC desc;
	pTextureInterface->GetDesc(&desc);

	mResourceManager->mSprites.back().second.mWidth = desc.Width;
	mResourceManager->mSprites.back().second.mHeight = desc.Height;

	KodeboldsMath::Vector2 origin;

	switch (pOrigin)
	{
	case SpriteOrigin::CENTRE:
		origin.X = float(desc.Width / 2);
		origin.Y = float(desc.Height / 2);
		break;
	}

	mResourceManager->mSprites.back().second.mOrigin = DirectX::XMFLOAT2(origin.X, origin.Y);
}
void GUIManager::LoadSprite(const wchar_t* pFileName, SpriteOrigin pOrigin, SpritePosition pPosition, KodeboldsMath::Vector2 pPadding, float pRotation, float pScale)
{
	Sprite sprite;
	sprite.mRotation = pRotation;
	sprite.mScale = pScale;

	mResourceManager->mSprites.emplace_back(std::make_pair(pFileName, sprite));

	auto imageTest = DirectX::CreateWICTextureFromFile(mDevice.Get(), mContext.Get(), pFileName, nullptr, mResourceManager->mSprites.back().second.mTexture.GetAddressOf());

	if (imageTest == 0x80070002) // file not found
	{
		int i = 0;
	}

	ID3D11Texture2D* pTextureInterface = 0;
	ID3D11Resource* res;
	mResourceManager->mSprites.back().second.mTexture.Get()->GetResource(&res);
	res->QueryInterface<ID3D11Texture2D>(&pTextureInterface);
	D3D11_TEXTURE2D_DESC desc;
	pTextureInterface->GetDesc(&desc);

	mResourceManager->mSprites.back().second.mWidth = desc.Width;
	mResourceManager->mSprites.back().second.mHeight = desc.Height;

	KodeboldsMath::Vector2 origin;

	switch (pOrigin)
	{
	case SpriteOrigin::CENTRE:
		origin.X = float(desc.Width / 2);
		origin.Y = float(desc.Height / 2);
		break;
	}

	mResourceManager->mSprites.back().second.mOrigin = DirectX::XMFLOAT2(origin.X, origin.Y);

	KodeboldsMath::Vector2 position;

	switch (pPosition)
	{
	case SpritePosition::CENTRE_TOP:
		position.X = (mDeviceWidth / 2.0f) + pPadding.X;
		position.Y = 0 + pPadding.Y;
		break;
	case SpritePosition::CENTRE_MIDDLE:
		position.X = (mDeviceWidth / 2.0f) + pPadding.X;
		position.Y = (mDeviceHeight / 2.0f) + pPadding.Y;
		break;
	case SpritePosition::CENTRE_BOTTOM:
		position.X = (mDeviceWidth / 2.0f) + pPadding.X;
		position.Y = mDeviceHeight + pPadding.Y;
		break;
	}

	mResourceManager->mSprites.back().second.mPosition = DirectX::XMFLOAT2(position.X, position.Y);
}

void GUIManager::LoadFont(const wchar_t* pFontName)
{
	mFonts.push_back(std::make_unique<DirectX::SpriteFont>(mDevice.Get(), pFontName));
}

void GUIManager::Write(const wchar_t* pText, KodeboldsMath::Vector2 pOrigin, KodeboldsMath::Vector2 pPosition, KodeboldsMath::Vector2 pPadding, const wchar_t* pFontName, float pRotation, float pScale, KodeboldsMath::Vector4 pColour)
{
	// TODO:
	// check if font has already been loaded
	// if not - load
	// if so = use it

	LoadFont(pFontName);

	Text text;
	text.mText = pText;
	text.mPosition = DirectX::XMFLOAT2(pPosition.X + pPadding.X, pPosition.Y + pPadding.Y);
	text.mOrigin = DirectX::XMFLOAT2(pOrigin.X, pOrigin.Y);
	text.mScale = pScale;
	text.mRotation = pRotation;
	text.mColour = DirectX::XMFLOAT4(pColour.X, pColour.Y, pColour.Z, pColour.W);

	mTexts.emplace_back(text);
}
void GUIManager::Write(const wchar_t* pText, KodeboldsMath::Vector2 pOrigin, TextPosition pPosition, KodeboldsMath::Vector2 pPadding, const wchar_t* pFontName, float pRotation, float pScale, KodeboldsMath::Vector4 pColour)
{
	KodeboldsMath::Vector2 position;

	switch (pPosition)
	{
	case TextPosition::CENTRE_TOP:
		position.X = (mDeviceWidth / 2.0f) + pPadding.X;
		position.Y = 0 + pPadding.Y;
		break;
	case TextPosition::CENTRE_MIDDLE:
		position.X = (mDeviceWidth / 2.0f) + pPadding.X;
		position.Y = (mDeviceHeight / 2.0f) + pPadding.Y;
		break;
	case TextPosition::CENTRE_BOTTOM:
		position.X = (mDeviceWidth / 2.0f) + pPadding.X;
		position.Y = mDeviceHeight + pPadding.Y;
		break;
	}

	Write(pText, pOrigin, position, pPadding, pFontName, pRotation, pScale, pColour);
}
void GUIManager::Write(const wchar_t* pText, TextOrigin pOrigin, KodeboldsMath::Vector2 pPosition, KodeboldsMath::Vector2 pPadding, const wchar_t* pFontName, float pRotation, float pScale, KodeboldsMath::Vector4 pColour)
{
	// TODO:
	// check if font has already been loaded
	// if not - load
	// if so = use it

	LoadFont(pFontName);

	Text text;
	text.mText = pText;
	text.mPosition = DirectX::XMFLOAT2(pPosition.X + pPadding.X, pPosition.Y + pPadding.Y);
	text.mScale = pScale;
	text.mRotation = pRotation;
	text.mColour = DirectX::XMFLOAT4(pColour.X, pColour.Y, pColour.Z, pColour.W);

	DirectX::XMFLOAT2 textSize;
	auto vecTextSize = mFonts[0]->MeasureString(pText);
	DirectX::XMStoreFloat2(&textSize, vecTextSize);

	switch (pOrigin)
	{
	case TextOrigin::CENTRE:
		text.mOrigin.x = float(textSize.x / 2);
		text.mOrigin.y = float(textSize.y / 2);
		break;
	}

	mTexts.emplace_back(text);
}
void GUIManager::Write(const wchar_t* pText, TextOrigin pOrigin, TextPosition pPosition, KodeboldsMath::Vector2 pPadding, const wchar_t* pFontName, float pRotation, float pScale, KodeboldsMath::Vector4 pColour)
{
	// TODO:
	// check if font has already been loaded
	// if not - load
	// if so = use it

	LoadFont(pFontName);

	Text text;
	text.mText = pText;
	text.mScale = pScale;
	text.mRotation = pRotation;
	text.mColour = DirectX::XMFLOAT4(pColour.X, pColour.Y, pColour.Z, pColour.W);

	switch (pPosition)
	{
	case TextPosition::CENTRE_TOP:
		text.mPosition.x = (mDeviceWidth / 2.0f) + pPadding.X;
		text.mPosition.y = 0 + pPadding.Y;
		break;
	case TextPosition::CENTRE_MIDDLE:
		text.mPosition.x = (mDeviceWidth / 2.0f) + pPadding.X;
		text.mPosition.y = (mDeviceHeight / 2.0f) + pPadding.Y;
		break;
	case TextPosition::CENTRE_BOTTOM:
		text.mPosition.x = (mDeviceWidth / 2.0f) + pPadding.X;
		text.mPosition.y = mDeviceHeight + pPadding.Y;
		break;
	}

	// origin
	DirectX::XMFLOAT2 textSize;
	auto vecTextSize = mFonts[0]->MeasureString(pText);
	DirectX::XMStoreFloat2(&textSize, vecTextSize);

	switch (pOrigin)
	{
	case TextOrigin::CENTRE:
		text.mOrigin.x = float(textSize.x / 2);
		text.mOrigin.y = float(textSize.y / 2);
		break;
	}

	mTexts.emplace_back(text);
}

