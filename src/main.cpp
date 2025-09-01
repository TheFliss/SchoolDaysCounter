
#include <extwinapi.h>

int main(int argc, char const *argv[])
{
    //const wchar_t *path = L"C:\\image.png";
    //int result;
    //result = SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (void *)path, SPIF_UPDATEINIFILE);
    //std::cout << result;        
    wchar_t *szWallpaperPath[MAX_PATH];
    FunctionHandlerL(SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, szWallpaperPath, 0), "SDC", "Cannot get current wallpaper path");
    cout << szWallpaperPath << endl;
    return 0;
}