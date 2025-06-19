#ifndef GAMEENGINE_FILE_UTIL
#define GAMEENGINE_FILE_UTIL

#include <d3d11.h>
#include <wincodec.h>
#include <vector>
#include <cassert>

void LoadTextureFromFile(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, const wchar_t* filename, ID3D11ShaderResourceView** textureView)
{
    // Initialize WIC
    IWICImagingFactory* wicFactory = nullptr;
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory));

    // Load the image from file
    IWICBitmapDecoder* decoder = nullptr;
    wicFactory->CreateDecoderFromFilename(filename, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);

    // Get the first frame of the image
    IWICBitmapFrameDecode* frame = nullptr;
    decoder->GetFrame(0, &frame);

    // Convert the image format to 32bpp RGBA
    IWICFormatConverter* converter = nullptr;
    wicFactory->CreateFormatConverter(&converter);
    converter->Initialize(frame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);

    // Get the size of the image
    UINT width, height;
    frame->GetSize(&width, &height);

    // Allocate a buffer to hold the image data
    UINT rowPitch = width * 4;  // 4 bytes per pixel (RGBA)
    std::vector<BYTE> imageData(rowPitch * height);
    converter->CopyPixels(nullptr, rowPitch, rowPitch * height, imageData.data());

    // Create a D3D11 texture from the image data with mipmap support
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 0;  // Enable all mip levels
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;  // Add render target for mipmap generation
    textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;  // Enable automatic mipmap generation

    // Create the texture without initial data (we'll update it separately)
    ID3D11Texture2D* texture = nullptr;
    d3dDevice->CreateTexture2D(&textureDesc, nullptr, &texture);

    HRESULT hr = d3dDevice->CreateShaderResourceView(texture, nullptr, textureView);
    if (FAILED(hr)) {
        assert(false);
    }

    // Update the top-level mip (level 0) with our image data
    d3dContext->UpdateSubresource(texture, 0, nullptr, imageData.data(), rowPitch, 0);

    // Generate the remaining mip levels automatically
    d3dContext->GenerateMips(*textureView);

    // Clean up
    texture->Release();
    converter->Release();
    frame->Release();
    decoder->Release();
    wicFactory->Release();
    CoUninitialize();
}

#endif // GAMEENGINE_FILE_UTIL