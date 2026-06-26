#include "DxgiCapturer.h"
#include <iostream>
#include <QDebug>

//初始化基于 DXGI Desktop Duplication API 的屏幕捕获器
bool DxgiCapturer::initialize(int width, int height, FrameCallback callback)
{
	_width = width;
	_height = height;
	_callback = callback;

	//创建一个硬件加速的 Direct3D 11 设备 (_device) 和立即执行上下文 (_context)
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
			D3D11_CREATE_DEVICE_BGRA_SUPPORT,&featureLevel, 1, D3D11_SDK_VERSION,
			&_dupDevice, nullptr, &_dupContext);
	if (FAILED(hr)) return false;

	//从 D3D11 设备逆向获取底层的 DXGI 对象，定位到具体的显示输出（显示器）
	ComPtr<IDXGIDevice> dxgiDevice;
	_dupDevice.As(&dxgiDevice);
	ComPtr<IDXGIAdapter> adapter;
	dxgiDevice->GetAdapter(&adapter);
	ComPtr<IDXGIOutput> output;
	adapter->EnumOutputs(0, &output);

	//调用 IDXGIOutput1::DuplicateOutput方法，创建桌面复制接口 (_duplication)
	ComPtr<IDXGIOutput1> output1;
	output.As(&output1);
	hr = output1->DuplicateOutput(_dupDevice.Get(), &_duplication);


	/*D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, 
		&featureLevel, 1, D3D11_SDK_VERSION, &_renderDevice, nullptr, &_renderContext);
		*/
	
	D3D11_TEXTURE2D_DESC sd = {};
	sd.Width = _width;
	sd.Height = _height;
	sd.MipLevels = 1;
	sd.ArraySize = 1;
	sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.SampleDesc.Count = 1;
	sd.Usage = D3D11_USAGE_STAGING;
	sd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	sd.BindFlags = 0;
	_dupDevice->CreateTexture2D(&sd, nullptr, &staging);

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = _width;
	desc.Height = _height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	_dupDevice->CreateTexture2D(&desc, nullptr, &_intermediate);

	_dupDevice->GetImmediateContext(&_dupContext);
	return SUCCEEDED(hr);
 }

void DxgiCapturer::captureFrame()
{
	if (!_duplication) return;

	DXGI_OUTDUPL_FRAME_INFO frameInfo;
	ComPtr<IDXGIResource> desktopResource;

	HRESULT hr = _duplication->AcquireNextFrame(50, &frameInfo, &desktopResource);
	if (hr == DXGI_ERROR_WAIT_TIMEOUT) return;
	if (FAILED(hr))
	{
		return;
	}
	DXGI_OUTDUPL_DESC dupDesc;
	_duplication->GetDesc(&dupDesc);
	qDebug() << "Dup:" << dupDesc.ModeDesc.Width << dupDesc.ModeDesc.Height << dupDesc.ModeDesc.Format;

	qDebug() << "AccumulatedFrames:" << frameInfo.AccumulatedFrames;
	ComPtr<ID3D11Texture2D> texture;
	desktopResource.As(&texture);

	D3D11_TEXTURE2D_DESC td;
	texture->GetDesc(&td);
	qDebug() << "Texture:" << td.Width << td.Height << td.Format << td.Usage << td.BindFlags;

	ID3D11Device* tDev = nullptr;
	texture->GetDevice(&tDev);
	qDebug() << "Texture Device:" << tDev;
	//qDebug() << "Render Device:"<<_renderDevice.Get();
	//qDebug() << "Same Device:" << (tDev == _renderDevice.Get());

	qDebug() << "Adapter Description:";
	ComPtr<IDXGIDevice> dxgiDevice;
	_dupDevice.As(&dxgiDevice);
	ComPtr<IDXGIAdapter> adapter;
	dxgiDevice->GetAdapter(&adapter);

	DXGI_ADAPTER_DESC desc;
	adapter->GetDesc(&desc);
	qDebug() << QString::fromWCharArray(desc.Description);

	if (_callback) {
		static uint64_t pts = 0;
		//_callback(texture.Get(), frameInfo.LastPresentTime.QuadPart);
		//_callback(texture.Get(), pts++);
		_dupContext->CopyResource(_intermediate, texture.Get());
		_dupContext->CopyResource(staging, _intermediate);
		_dupContext->Flush();
		_callback(staging, pts++);
	}

	_duplication->ReleaseFrame();
}

void DxgiCapturer::release()
{
	if (_duplication) {
		_duplication->ReleaseFrame();
		_duplication.Reset();
	}
}