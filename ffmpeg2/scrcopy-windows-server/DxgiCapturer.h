#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <functional>

using Microsoft::WRL::ComPtr;

class DxgiCapturer
{
public:
	using FrameCallback = std::function<void(ID3D11Texture2D*, UINT64)>;

	bool initialize(int width, int height, FrameCallback callback);

	void captureFrame();

	void release();

	ID3D11Device* getDevice() { return _dupDevice.Get(); }
private:
	ComPtr<IDXGIOutputDuplication> _duplication;
	ComPtr<ID3D11Device> _dupDevice;
	ComPtr<ID3D11DeviceContext> _dupContext;
	//ComPtr<ID3D11Device> _renderDevice;
	//ComPtr<ID3D11DeviceContext> _renderContext;
	FrameCallback _callback;
	int _width, _height;
	ID3D11Texture2D* staging = nullptr;
	ID3D11Texture2D* _intermediate = nullptr;
};

