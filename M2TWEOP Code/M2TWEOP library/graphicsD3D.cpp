#include "pch.h"
#include "graphicsD3D.h"
#include "plugins.h"
#include <DxErr.h>
#pragma comment(lib, "DXERR.lib")

#include "eopImgui.h"
#include "imgui_notify.h"

#include <d3d9.h>
#include <d3dx9.h>

#include "fbxModels.h"


#include <ImFileDialog.h>
//#include "discord.h"
graphicsD3D::dataT graphicsD3D::dataS;

#include "onlineThings.h"

#include "m2tweopMapManager.h"

#include "../../M2TWEOP Common/m2tweopConstData.h"
#include "eduFastFuncts.h"
template<typename T>
T FnCast(uint32_t fnToCast, T pFnCastTo) {
	(void)pFnCastTo;
	return (T)fnToCast;
}
NOINLINE LRESULT APIENTRY graphicsD3D::hkWndProc2(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	plugins::onWindowProc(hWnd, uMsg, wParam, lParam);

	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	{
		return true;
	}
	// TO-DO: Stop this code from crashing
	// switch (uMsg)
	// {
	// case WM_SETFOCUS:
	// 	RECT rect;
	// 	if (!GetWindowRect(hWnd, &rect)) {
	// 		break;
	// 	}
	// 	ClipCursor(&rect);
	// 	break;
	// case WM_KILLFOCUS:
	// 	ClipCursor(NULL);
	// 	break;
	// }
	if (dataS.ifMouseOrKeyBoardAtImgui)
	{
		switch (uMsg)
		{
		case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
		case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
		case WM_MOUSEHWHEEL:
		case WM_MOUSEWHEEL:
		case WM_CHAR:
		case WM_SETCURSOR:
		case WM_MOUSEMOVE:
		case WM_INPUTLANGCHANGE:
		case WM_KEYDOWN:
		case WM_KEYUP:
			return true;
		}

	}

	return CallWindowProc(dataS.hookD.onewGameWndProc, hWnd, uMsg, wParam, lParam);
}

struct
{
	float drawInfoEndTime = 0;
	bool drawEOPStartInfo=false;

	ImVec2 beginCoords{ 0.f,0.f };
}drawParams;

struct {
	//std::unique_ptr<discord::Core> core;
}DiscordState;

struct
{
	std::vector<IDirect3DTexture9*>texturesForDeleting;
}tempData;
NOINLINE void graphicsD3D::Draw(LPDIRECT3DDEVICE9 pDevice)
{
//	DiscordState.core->RunCallbacks();


	/*MEMORYSTATUSEX statex;

	statex.dwLength = sizeof(statex);

	GlobalMemoryStatusEx(&statex);
	ImGui::Begin("Ram test");
	ImGui::Text("There are %I64d total MB of virtual memory.\n",
		statex.ullTotalVirtual / 1048576);
	ImGui::Text("There are %I64d free  MB of virtual memory.\n",
		statex.ullAvailVirtual / 1048576);

	static float progress = 0.0f, progress_dir = 1.0f;


	float prop =1.0f- (double)statex.ullAvailVirtual / (double)statex.ullTotalVirtual;

	ImGui::Text("Memory usage:");
	ImGui::ProgressBar(prop, ImVec2(-1.0f, 0.0f));

	ImGui::End();*/


	/*ImGui::Begin("test");

	if (ImGui::Button("fight"))
	{
		_asm
		{
			push 1
			mov ecx, 0x02BF8550
			mov eax,0x0044edb0
			call eax
			add esp, 4
		}
	}
	ImGui::End();*/
	return;
}
NOINLINE void graphicsD3D::onDrawPartsOfStratObjects()
{
	//plugins::onEndScene(pDevice);


	//Backup the DX9 state
	IDirect3DStateBlock9* d3d9_state_block = NULL;
	if (graphicsD3D::dataS.pDevice->CreateStateBlock(D3DSBT_ALL, &d3d9_state_block) < 0)
		return;
	if (d3d9_state_block->Capture() < 0)
	{
		d3d9_state_block->Release();
		return;
	}
	// Backup the DX9 transform (DX9 documentation suggests that it is included in the StateBlock but it doesn't appear to)
	D3DMATRIX last_world, last_view, last_projection;
	graphicsD3D::dataS.pDevice->GetTransform(D3DTS_WORLD, &last_world);
	graphicsD3D::dataS.pDevice->GetTransform(D3DTS_VIEW, &last_view);
	graphicsD3D::dataS.pDevice->GetTransform(D3DTS_PROJECTION, &last_projection);


	graphicsD3D::dataS.clearStateBlock->Apply();



	int battleState = smallFuncs::getGameDataAll()->battleHandler->battleState;

	//1-stratmap
	//2-tactmap
	int drawType = 2;
	if (battleState == 0)
	{
		drawType = 1;
	}
	graphicsD3D::dataS.pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	graphicsD3D::dataS.pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	graphicsD3D::dataS.pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	fbxModels::draw(drawType, globals::dataS.gamever);



	// Restore the DX9 transform
	graphicsD3D::dataS.pDevice->SetTransform(D3DTS_WORLD, &last_world);
	graphicsD3D::dataS.pDevice->SetTransform(D3DTS_VIEW, &last_view);
	graphicsD3D::dataS.pDevice->SetTransform(D3DTS_PROJECTION, &last_projection);

	// Restore the DX9 state
	d3d9_state_block->Apply();
	d3d9_state_block->Release();

}
void graphicsD3D::onDrawAllGameStuff()
{
	if (graphicsD3D::dataS.pDevice->BeginScene() < 0)
	{
		return;
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	auto& developerMode = globals::dataS.Modules.developerMode;
	developerMode.Update();
	plugins::onEndScene(graphicsD3D::dataS.pDevice);
	//graphicsD3D::Draw(pDevice);
	if (drawParams.drawEOPStartInfo == true)
	{
		static IDirect3DTexture9* logoEOP = nullptr;
		static int logoX = 0;
		static int logoY = 0;
		float currTime = (float)ImGui::GetTime();

		if (currTime < drawParams.drawInfoEndTime)
		{
			if (logoEOP == nullptr)
			{
				//logoEOP=graphicsExport::loadTexture(std::string(globals::dataS.modPatch+ "/eopData/images/logoAbout.png").c_str(),&logoX,&logoY);
				//ImGui::OpenPopup("M2TWEOP");
			}

			static ImGuiWindowFlags transparentF = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;

			ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
			ImGui::Begin("eopInitTitle", nullptr, transparentF);

			ImGui::Text(eopConstData::eopVersionName);

			ImGui::End();


			if (logoEOP != nullptr)
			{
				ImVec2 logoPos = ImGui::GetMainViewport()->Size;
				logoPos.x /= 2;
				logoPos.y *= 0.1f;


				ImGui::SetNextWindowPos(logoPos, ImGuiCond_Always, ImVec2(0.5f, 0.0f));
				ImGui::SetNextWindowSize(ImVec2(logoPos.x, -1), ImGuiCond_Always);
				if (ImGui::BeginPopupModal("M2TWEOP", nullptr, transparentF))
				{

					ImVec2 const csz = ImGui::GetContentRegionAvail();
					ImVec2 logoSize = ImVec2((float)logoX, (float)logoY);
					if (logoSize.x > csz.x) {
						const float r = logoSize.y / logoSize.x;
						logoSize.x = csz.x;
						logoSize.y = csz.x * r;
					}
					ImGui::Image((void*)(intptr_t)logoEOP, logoSize);

					ImGui::NewLine();
					ImGui::NewLine();
					ImGui::NewLine();
					ImGui::SetCursorPosX((ImGui::GetWindowSize().x - logoPos.y * 2) * 0.5f);
					ImGui::LoadingIndicatorCircle("##loadCircle1", logoPos.y, ImVec4(0.584, 0.270, 0.250, 1.0f), ImVec4(0.564, 0.250, 0.230, 1.0f), 15, 5);

					ImGui::NewLine();
					ImGui::NewLine();
					ImGui::NewLine();

					if (currTime >= drawParams.drawInfoEndTime)
					{
						ImGui::CloseCurrentPopup();
						tempData.texturesForDeleting.push_back(logoEOP);
					}
					ImGui::EndPopup();
				}
			}
		}
		else
		{
			drawParams.drawEOPStartInfo = false;
			ImGuiToast bMsg(ImGuiToastType_Success, 25000);

			bMsg.set_title("Initializing M2TWEOP");
			bMsg.set_content("Done");
			ImGui::InsertNotification(bMsg);
		}
	}


	battleCreator::draw(graphicsD3D::dataS.pDevice);


	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.f);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(43.f / 255.f, 43.f / 255.f, 43.f / 255.f, 100.f / 255.f));
	ImGui::RenderNotifications();
	ImGui::PopStyleVar(1);
	ImGui::PopStyleColor(1);



	ImGuiIO& io = ImGui::GetIO();

	graphicsD3D::dataS.ifMouseOrKeyBoardAtImgui = ImGui::GetIO().WantCaptureMouse;
	graphicsD3D::dataS.ifMouseOrKeyBoardAtImgui |= ImGui::GetIO().WantCaptureKeyboard;



	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	if (tempData.texturesForDeleting.size())
	{
		for (auto tex : tempData.texturesForDeleting)
		{
			tex->Release();
		}
		tempData.texturesForDeleting.clear();
	}


	graphicsD3D::dataS.pDevice->EndScene();
}
NOINLINE LRESULT APIENTRY graphicsD3D::hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return CallWindowProc(dataS.hookD.oWndProc, hWnd, uMsg, wParam, lParam);
}


NOINLINE void graphicsD3D::initImgGui(IDirect3DDevice9* pDevice)
{



	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
	io.MouseDrawCursor = false;
	string f = globals::dataS.modPatch;
	if (f.size() == 0)
	{
		MessageBoxA(NULL, "graphics init error", "Error", NULL);
		exit(0);
	}
	f = f + "\\eopData\\fonts\\mainFont.ttf";

	ImGui::StyleGrey();


	ImGui_ImplWin32_Init(dataS.Window);
	ImGui_ImplDX9_Init(pDevice);


	ImFontConfig font_config;
	font_config.OversampleH = 1;
	font_config.OversampleV = 1;
	font_config.SizePixels = 12.f;
	font_config.PixelSnapH = 1;
	font_config.FontDataOwnedByAtlas = false;

	ImFont* newFont = io.Fonts->AddFontFromFileTTF(f.c_str(), 16.f, &font_config, io.Fonts->GetGlyphRangesCyrillic());
	if (newFont == nullptr)
	{
		io.Fonts->AddFontDefault(&font_config);
	}
	//init imnotify
	ImGui::MergeIconsWithLatestFont(16.f, false);

	dataS.ImInitialized = true;

	drawParams.drawEOPStartInfo = true;
	drawParams.drawInfoEndTime = (float)ImGui::GetTime()+20.0f;


	return;
}

bool graphicsD3D::init()
{
	IMGUI_CHECKVERSION();
	ImGuiContext* imCtx = ImGui::CreateContext();
	plugins::onChangeImGuiCtx(imCtx);


	while (dataS.Window == nullptr)
	{
		dataS.Window = FindWindowA(0, ("Medieval 2"));
		if (dataS.Window) break;
		Sleep(20);
	}

	dataS.hookD.oWndProc = (WNDPROC)SetWindowLongA(dataS.Window, GWL_WNDPROC, (LONG)hkWndProc);

	if (dataS.hookD.oWndProc == nullptr) return false;

	return true;
}



DWORD __stdcall graphicsD3D::InitS()
{
	/* Initialize MinHook.
	if (MH_Initialize() != MH_OK)
	{
		return 1;
	}*/

	init();

	DWORD adr = 0;
	if (globals::dataS.gamever == 2)//steam
	{
		adr = 0x01178a40;
	}
	else
	{
		adr = 0x011780d0;
	}

	if (MH_CreateHook((char*)adr, &hkWndProc2,
		reinterpret_cast<LPVOID*>(&dataS.hookD.onewGameWndProc)) != MH_OK)
	{
		return 1;
	}
	if (MH_EnableHook((char*)adr) != MH_OK)
	{
		return 1;
	}



	return 1;
}


NOINLINE EOP_EXPORT LPDIRECT3DTEXTURE9 graphicsExport::loadTexture(const char* path, int* x, int* y)
{
	LPDIRECT3DTEXTURE9 imageRet = nullptr;
	HRESULT res = D3DXCreateTextureFromFileA(graphicsD3D::dataS.pDevice, path, &imageRet);
	if (res != D3D_OK || imageRet == nullptr)
	{
		std::string errMes = DXGetErrorStringA(res);
		errMes += "\n";
		errMes += path;
		MessageBoxA(NULL, errMes.c_str(), "Loading texture err!", MB_OK | MB_ICONASTERISK);
		return nullptr;
	}

	D3DSURFACE_DESC my_image_desc;

	imageRet->GetLevelDesc(0, &my_image_desc);

	*x = (int)my_image_desc.Width;
	*y = (int)my_image_desc.Height;

	return imageRet;
}

NOINLINE EOP_EXPORT void graphicsExport::unloadTexture(LPDIRECT3DTEXTURE9 texture)
{
	tempData.texturesForDeleting.push_back(texture);
}

NOINLINE EOP_EXPORT void graphicsExport::onCreateDevice(IDirect3DDevice9* pDevice)
{
	graphicsD3D::dataS.pDevice = pDevice;

	graphicsD3D::InitS();


	graphicsD3D::initImgGui(pDevice);


	/*discord::Core* core{};
	auto response = discord::Core::Create(879470336565981186, DiscordCreateFlags_Default, &core);
	DiscordState.core.reset(core);

	if (!DiscordState.core) {
	//	std::cout << "Failed to instantiate Discord!";
		std::exit(-1);
	}

	discord::Activity activity{};
	activity.SetDetails("Medieval II total war");
	activity.SetState("M2TWEOP");
	activity.GetAssets().SetSmallImage("test.png");
	activity.GetAssets().SetSmallText("123321");
	activity.GetAssets().SetLargeImage("test.png");
	activity.GetAssets().SetLargeText("321123");
	activity.SetType(discord::ActivityType::Playing);
	DiscordState.core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
	//	std::cout << ((result == discord::Result::Ok) ? "Succeeded" : "Failed")
		//	<< " updating activity!\n";
		});
	*/


	fbxModels::set3dDevice(pDevice);



	//fbxModels::addFbxModel("scorpid.fbx", "scorp.dds",1);
	//fbxModels::addFbxModel("Tiger.fbx", "FbxTemp_0001.jpg",3);
	//fbxModels::addFbxModel("Intergalactic_Spaceship-(FBX 6.1 ASCII).fbx", "Intergalactic Spaceship_rough.jpg",5);


	//void* obj=fbxModels::addFbxObject(1,2);
	//void* obj2=fbxModels::addFbxObject(3,4);
	//void* obj3=fbxModels::addFbxObject(5,6);

	//float coords[3]{ 0,0,0 };
	//float coords2[3]{ 2,0,2 };
	//float coords3[3]{ 4,5,4 };
	//fbxModels::setFbxObjectCoords(obj, coords);
	//fbxModels::setFbxObjectCoords(obj2, coords2);
	//fbxModels::setFbxObjectCoords(obj3, coords3);
	//fbxModels::setFbxObjectSize(obj2,0.007);
	//fbxModels::setFbxObjectSize(obj3,0.003);


	ifd::FileDialog::Instance().CreateTexture = [](uint8_t* data, int w, int h, char fmt) -> void* {
		IDirect3DTexture9* texture = NULL;

		// Create empty IDirect3DTexture9*
		if (fmt)
		{
			graphicsD3D::dataS.pDevice->CreateTexture(w, h, 1, 0,
				D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &texture, 0);
		}
		else
		{
			graphicsD3D::dataS.pDevice->CreateTexture(w, h, 1, 0,
				D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &texture, 0);

		}
		if (!texture)
		{
			throw std::runtime_error("CreateTexture failed");
		}

		D3DLOCKED_RECT rect;
		texture->LockRect(0, &rect, 0, D3DLOCK_DISCARD);
		unsigned char* dest = static_cast<unsigned char*>(rect.pBits);
		memcpy(dest, &data[0], sizeof(unsigned char) * w * h * 4);
		texture->UnlockRect(0);

		return (void*)texture;
	};

	ifd::FileDialog::Instance().DeleteTexture = [](void* tex) {
		tempData.texturesForDeleting.push_back((IDirect3DTexture9*)tex);
	};
}

NOINLINE EOP_EXPORT void graphicsExport::onEndScene(IDirect3DDevice9* pDevice)
{

}

NOINLINE EOP_EXPORT void graphicsExport::onReset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	ImGui_ImplDX9_InvalidateDeviceObjects();


	if (graphicsD3D::dataS.clearStateBlock != NULL)
		graphicsD3D::dataS.clearStateBlock->Release();
}

NOINLINE EOP_EXPORT void graphicsExport::afterReset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	ImGui_ImplDX9_CreateDeviceObjects();

	pDevice->CreateStateBlock(D3DSBT_ALL, &graphicsD3D::dataS.clearStateBlock);
}
