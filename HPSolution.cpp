// HPSolution.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "HPSolution.h"

using namespace std;

#define MAX_LOADSTRING 100

//
// Global Variables:
//
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND hwndCalculate;  //calculate helium button 
HWND hwndChoose;  //choose file button
HWND hwndCalculateGas;  //calc gas button
HWND hwndSave;  //save results
HWND hwndLoadHelium;  //load helium data
HWND hwndLoadGas;  //load gas data
HWND hwndAdd;  //append extra gas data
HWND massEnter;  //enter mass dialog
HWND massLoad;  //enter mass dialog
HWND static_in;
HWND static_out;
HWND static_out1;
PWSTR filePath;
string strFilePath;
int fileFlag = 0;
double massBuff = 0;

//graphics coordinates and sizes
int buttonHeight = 40;
int buttonLength = 100;
int firstRow = 10;
int secondRow = 210;
int thirdRow = 260;
int firstColumn = 10;
int closeSpacing = 5;
int dataWindow = 100;

//
//Class definitions
//

class classSorptionData
{
public:
	//input variables
	double mass; //grams
	double Vsc;  //cc
	double Vrc;
	double Vc;

	//process variables
	int heliumSteps;
	int gasSteps;

	//constants
	const double R = 8.314462;
	const double MCH4 = 16;

	vector<vector<std::string>> heliumRaw;
	vector<vector<std::string>> gasRaw;

	//derived variables
	double Vvoid;
	vector<vector<double>> isotherm;

	//functions

	double heliumCalibration(classSorptionData sorptionData); //main helium calibration processing
	vector<vector<double>> gasSorption(classSorptionData sorptionData); //main gas sorption processing function

	double Z_VdW_He(double T, double P); //Van der Waals equiation for real gas factor, Helium
	double Z_PR_CH4(double T, double P, classSorptionData sorptionData); //Peng-Robinson equiation of state, methane
	double cardano_min(double a, double b, double c, double d); //cubic equiation solution

};

//temporary object definition - to be moved in specific tools
classSorptionData sorptionData;

class CSVReader
{
	std::string fileName;
	std::string delimeter;
public:
	CSVReader(std::string filename, std::string delm = ";") :
		fileName(filename), delimeter(delm)
	{ }
	// Function to fetch data from a CSV File
	std::vector<std::vector<std::string> > getData();
};

//
// Forward declarations of functions included in this code module:
//
ATOM                MyRegisterClass(HINSTANCE hInstance);
HWND                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
wchar_t getFilePath(HWND hWnd, HINSTANCE hInst);
std::vector<std::vector<std::string> > csvRead(PWSTR filePath);
string getSavePathWithDialog();
string getLoadPathWithDialog();
void csvWrite(string savePath, classSorptionData sorptionData);
bool is_number(const std::string& s);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
	//temporary object and attrubites definition - to be moved in specific tools
	sorptionData.mass = 4.9891;
	sorptionData.Vrc = 5.134534433;
	sorptionData.Vsc = 19.520139;
	sorptionData.heliumSteps = 6;
	sorptionData.Vvoid = 0;

	

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_HPSOLUTION, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

	HWND hWnd = InitInstance(hInstance, nCmdShow);

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_HPSOLUTION));

	//GUI elements
	 hwndCalculate = CreateWindow(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"Calculate Void Volume",      // Button text 
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		firstColumn,         // x position 
		thirdRow,         // y position 
		2*buttonLength,        // Button width
		buttonHeight,        // Button height
		hWnd,     // Parent window
		(HMENU) IDC_CALCULATE,       // No menu.
		(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		NULL);      // Pointer not needed.
	 static_out = CreateWindow(L"Static",
		 L"Void Voilume not Calculated",
		 WS_CHILD | WS_VISIBLE,
		 firstColumn, (thirdRow + buttonHeight + closeSpacing), 2*buttonLength, dataWindow, hWnd, 0, hInst, 0);

	 hwndCalculateGas = CreateWindow(
		 L"BUTTON",  // Predefined class; Unicode assumed 
		 L"Calculate Isotherm",      // Button text 
		 WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		 (firstColumn + 2*buttonLength + closeSpacing),         // x position 
		 thirdRow,         // y position 
		 150,        // Button width
		 buttonHeight,        // Button height
		 hWnd,     // Parent window
		 (HMENU)IDC_CALCULATE_GAS,       // No menu.
		 (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		 NULL);      // Pointer not needed.
	 static_out = CreateWindow(L"Static",
		 L"Isotherm not Calculated",
		 WS_CHILD | WS_VISIBLE,
		 (firstColumn + 2 * buttonLength + closeSpacing), (thirdRow + buttonHeight + closeSpacing), 2 * buttonLength, dataWindow, hWnd, 0, hInst, 0);

	 hwndSave = CreateWindow(
		 L"BUTTON",  // Predefined class; Unicode assumed 
		 L"Save",      // Button text 
		 WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		 (firstColumn + 2 * buttonLength + closeSpacing + 150 + 2*closeSpacing),         // x position 
		 thirdRow,         // y position 
		 40,        // Button width
		 buttonHeight,        // Button height
		 hWnd,     // Parent window
		 (HMENU)IDC_SAVE,       // No menu.
		 (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		 NULL);      // Pointer not needed.
	 static_out = CreateWindow(L"Static",
		 L"No Save Directory/File Selected",
		 WS_CHILD | WS_VISIBLE,
		 (firstColumn + 4 * buttonLength + 2 * closeSpacing), thirdRow, 4 * buttonLength, buttonHeight, hWnd, 0, hInst, 0);

	 hwndChoose = CreateWindow(
		 L"BUTTON",  // Predefined class; Unicode assumed 
		 L"Choose File",      // Button text 
		 WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		 firstColumn,         // x position 
		 firstRow,         // y position 
		 (4*buttonLength + closeSpacing),        // Button width
		 buttonHeight,        // Button height
		 hWnd,     // Parent window
		 (HMENU) IDC_CHOOSE,       // No menu.
		 (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		 NULL);      // Pointer not needed.
	 static_out = CreateWindow(L"Static",
		 L"No File Selected",
		 WS_CHILD | WS_VISIBLE,
		 (firstColumn + 4 * buttonLength + 2 * closeSpacing), firstRow, 4 * buttonLength, buttonHeight, hWnd, 0, hInst, 0);

	 hwndLoadHelium = CreateWindow(
		 L"BUTTON",  // Predefined class; Unicode assumed 
		 L"Load Helium Data",      // Button text 
		 WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		 firstColumn,         // x position 
		 (firstRow + buttonHeight + closeSpacing),         // y position 
		 (2*buttonLength),        // Button width
		 buttonHeight,        // Button height
		 hWnd,     // Parent window
		 (HMENU)IDC_LOAD,       // No menu.
		 (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		 NULL);      // Pointer not needed.
	 static_out = CreateWindow(L"Static",
		 L"No Helium Data Loaded",
		 WS_CHILD | WS_VISIBLE,
		 firstColumn, (firstRow + 2 * buttonHeight + 2 * closeSpacing), 2 * buttonLength, dataWindow, hWnd, 0, hInst, 0);

	 hwndLoadGas = CreateWindow(
		 L"BUTTON",  // Predefined class; Unicode assumed 
		 L"Load Gas Data",      // Button text 
		 WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		 (firstColumn + 2*buttonLength + closeSpacing),         // x position 
		 (firstRow + buttonHeight + closeSpacing),         // y position 
		 150,        // Button width
		 buttonHeight,        // Button height
		 hWnd,     // Parent window
		 (HMENU)IDC_LOAD_GAS,       // No menu.
		 (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		 NULL);      // Pointer not needed.
	 static_out = CreateWindow(L"Static",
		 L"No Gas Data Loaded",
		 WS_CHILD | WS_VISIBLE,
		 (firstColumn + 2*buttonLength + closeSpacing), (firstRow + 2 * buttonHeight + 2 * closeSpacing), 2 * buttonLength, dataWindow, hWnd, 0, hInst, 0);

	 hwndAdd = CreateWindow(
		 L"BUTTON",  // Predefined class; Unicode assumed 
		 L"Add",      // Button text 
		 WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		 (firstColumn + 2 * buttonLength + closeSpacing + 150 + 2*closeSpacing),         // x position 
		 (firstRow + buttonHeight + closeSpacing),         // y position 
		 40,        // Button width
		 buttonHeight,        // Button height
		 hWnd,     // Parent window
		 (HMENU)IDC_LOAD_GAS_EXTRA,       // No menu.
		 (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		 NULL);      // Pointer not needed.

	 static_out = CreateWindow(L"Static",
		 L"Mass [g]:",
		 WS_CHILD | WS_VISIBLE,
		 (firstColumn), (secondRow), 70, buttonHeight, hWnd, 0, hInst, 0);

	massEnter = CreateWindow(
		L"EDIT",  // Predefined class; Unicode assumed 
		L"",      // Button text 
		WS_BORDER | WS_VISIBLE | WS_CHILD ,  // Styles 
		firstColumn + 70 + closeSpacing,         // x position 
		secondRow,         // y position 
		2*buttonLength,        // Button width
		buttonHeight,        // Button height
		hWnd,     // Parent window
		(HMENU)IDD_MASS,       // No menu.
		(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		NULL);      // Pointer not needed.
	massLoad = CreateWindow(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"Use this Mass",      // Button text 
		WS_BORDER | WS_VISIBLE | WS_CHILD,  // Styles 
		firstColumn + 70 + 2*buttonLength + 2*closeSpacing,         // x position 
		secondRow,         // y position 
		buttonLength + 25,        // Button width
		buttonHeight,        // Button height
		hWnd,     // Parent window
		(HMENU)IDC_MASS_LOAD,       // No menu.
		(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		NULL);      // Pointer not needed.
	static_out = CreateWindow(L"Static",
		L"Default Example Mass is used",
		WS_CHILD | WS_VISIBLE,
		(firstColumn + 70 + 3 * buttonLength + 3 * closeSpacing + 25), (secondRow), 150, buttonHeight, hWnd, 0, hInst, 0);


    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HPSOLUTION));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_HPSOLUTION);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return hWnd;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	
    switch (message)
    {
	case WM_CREATE:
	{
		
		break;
	}
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;

			case IDC_CHOOSE:
			{
				/*getFilePath(hWnd, hInst);
				fileFlag++;*/

				strFilePath = getLoadPathWithDialog();

				int bufferlen = MultiByteToWideChar(CP_ACP, 0, strFilePath.c_str(), strFilePath.size(), NULL, 0);
				filePath = new WCHAR[bufferlen + 1];
				MultiByteToWideChar(CP_ACP, 0, strFilePath.c_str(), strFilePath.size(), filePath, bufferlen);



				static_out = CreateWindow(L"Static",
					filePath,
					WS_CHILD | WS_VISIBLE,
					(firstColumn + 4*buttonLength + 2*closeSpacing), firstRow, 4*buttonLength, buttonHeight, hWnd, 0, hInst, 0);
				MessageBoxW(NULL, L"File Selected", L"File Path", MB_OK);
				break;
			}
			case IDC_SAVE:
			{
				if (sorptionData.isotherm.size() != 0)
				{
					string strSavePath = getSavePathWithDialog();

					int bufferlen = MultiByteToWideChar(CP_ACP, 0, strSavePath.c_str(), strSavePath.size(), NULL, 0);
					LPWSTR savePath = new WCHAR[bufferlen + 1];
					MultiByteToWideChar(CP_ACP, 0, strSavePath.c_str(), strSavePath.size(), savePath, bufferlen);

					static_out = CreateWindow(L"Static",
						savePath,
						WS_CHILD | WS_VISIBLE,
						(firstColumn + 4 * buttonLength + 2 * closeSpacing), thirdRow, 4 * buttonLength, buttonHeight, hWnd, 0, hInst, 0);
					csvWrite(strSavePath, sorptionData);
					MessageBoxW(NULL, L"Results Saved!", L"Results Save", MB_OK);
				}
				else
				{
					MessageBoxW(NULL, L"No Data to Save!", L"Results Save", MB_OK);
				}
				break;
			}
			case IDC_LOAD:
			{
				sorptionData.heliumRaw.clear();
				if (filePath != nullptr)
				{
					
					sorptionData.heliumRaw = csvRead(filePath);
					std::string firstLine;
					for (std::string data : sorptionData.heliumRaw[0])
					{
						firstLine = firstLine + data + " , ";
					}
					
					int bufferlen = MultiByteToWideChar(CP_ACP, 0, firstLine.c_str(), firstLine.size(), NULL, 0);
					LPWSTR charFirstLine = new WCHAR[bufferlen + 1];
					MultiByteToWideChar(CP_ACP, 0, firstLine.c_str(), firstLine.size(), charFirstLine, bufferlen);
					static_out = CreateWindow(L"Static",
						charFirstLine,
						WS_CHILD | WS_VISIBLE,
						firstColumn, (firstRow + 2*buttonHeight + 2*closeSpacing), 2*buttonLength, dataWindow, hWnd, 0, hInst, 0);
					MessageBoxW(NULL, L"Data Loaded", L"Data Load", MB_OK);
					break;
				}
				else
				{
					MessageBoxW(NULL, L"No File Selected!", L"Data Load", MB_OK);
					break;
				}
			}
			case IDC_LOAD_GAS:
			{
				sorptionData.gasRaw.clear();
				if (filePath != nullptr)
				{
					
					sorptionData.gasRaw = csvRead(filePath);
					std::string firstLine;
					for (std::string data : sorptionData.gasRaw[0])
					{
						firstLine = firstLine + data + " , ";
					}

					int bufferlen = MultiByteToWideChar(CP_ACP, 0, firstLine.c_str(), firstLine.size(), NULL, 0);
					LPWSTR charFirstLine = new WCHAR[bufferlen + 1];
					MultiByteToWideChar(CP_ACP, 0, firstLine.c_str(), firstLine.size(), charFirstLine, bufferlen);
					static_out = CreateWindow(L"Static",
						charFirstLine,
						WS_CHILD | WS_VISIBLE,
						(firstColumn + 2 * buttonLength + closeSpacing), (firstRow + 2 * buttonHeight + 2 * closeSpacing), 2 * buttonLength, dataWindow, hWnd, 0, hInst, 0);
					MessageBoxW(NULL, L"Data Loaded", L"Data Load", MB_OK);
					break;
				}
				else
				{
					MessageBoxW(NULL, L"No File Selected!", L"Data Load", MB_OK);
					break;
				}
			}
			case IDC_MASS_LOAD:
			{
				const HWND text_box = GetDlgItem(hWnd, IDD_MASS);
				const int n = GetWindowTextLength(text_box);
				wstring text(n + 1, L'#');
				if (n > 0)
				{
					GetWindowText(text_box, &text[0], text.length());
				}
				text.resize(n);  //wstring with the entered shit
				
				char massBuff_char[1024];
				char DefChar = ' ';
				WideCharToMultiByte(CP_ACP, 0, text.c_str(), -1, massBuff_char, 1024, &DefChar, NULL);
				//A std:string  using the char* constructor.
				std::string strMassBuff(massBuff_char);

				if (is_number(strMassBuff))
				{
					massBuff = stod(strMassBuff);
					static_out = CreateWindow(L"Static",
						text.c_str(),
						WS_CHILD | WS_VISIBLE,
						(firstColumn + 70 + 3 * buttonLength + 3 * closeSpacing + 25), (secondRow), 150, buttonHeight, hWnd, 0, hInst, 0);
					MessageBox(hWnd, text.c_str(), L"Mass:", MB_OK);

				}
				else
				{
					MessageBox(hWnd, L"Not a Number!", L"Mass:", MB_OK);
				}
				break;
			}
			case IDC_CALCULATE:
			{
				if (sorptionData.heliumRaw.size() != 0)
				{
					
					sorptionData.Vvoid = sorptionData.heliumCalibration(sorptionData);

					float whole, fractional;
					fractional = 10000*std::modf(sorptionData.Vvoid, &whole);
					string stemp = "V0 = " + to_string((int)whole) + "." + to_string((int)fractional) + " cm3";
					int bufferlen = MultiByteToWideChar(CP_ACP, 0, stemp.c_str(), stemp.size(), NULL, 0);
					LPWSTR charTemp = new WCHAR[bufferlen + 1];
					MultiByteToWideChar(CP_ACP, 0, stemp.c_str(), stemp.size(), charTemp, bufferlen);
					static_out = CreateWindow(L"Static",
						charTemp,
						WS_CHILD | WS_VISIBLE,
						firstColumn, (thirdRow + buttonHeight + closeSpacing), 2 * buttonLength, dataWindow, hWnd, 0, hInst, 0);
				}
				else
				{
					MessageBoxW(NULL, L"No Data Loaded!", L"Helium Calibration", MB_OK);
					break;
				}
				break;
			}
			case IDC_CALCULATE_GAS:
			{
				if (massBuff != 0)
				{
					sorptionData.mass = massBuff;
					MessageBoxW(NULL, L"Using User-Provided Mass", L"Gas Sorption", MB_OK);
				}
				else
				{
					MessageBoxW(NULL, L"Using Default Example Mass (4.9891g)", L"Gas Sorption", MB_OK);
				}

				if ((sorptionData.gasRaw.size() != 0) && (sorptionData.Vvoid != 0))
				{
					
					sorptionData.isotherm = sorptionData.gasSorption(sorptionData);

					//float whole, fractional;
					//fractional = 10000 * std::modf(sorptionData.Vvoid, &whole);
					//string stemp = "V0 = " + to_string((int)whole) + "." + to_string((int)fractional) + " cm3";
					//int bufferlen = MultiByteToWideChar(CP_ACP, 0, stemp.c_str(), stemp.size(), NULL, 0);
					//LPWSTR charTemp = new WCHAR[bufferlen + 1];
					//MultiByteToWideChar(CP_ACP, 0, stemp.c_str(), stemp.size(), charTemp, bufferlen);
					static_out = CreateWindow(L"Static",
						L"Done!",
						WS_CHILD | WS_VISIBLE,
						(firstColumn + 2 * buttonLength + closeSpacing), (thirdRow + buttonHeight + closeSpacing), 2 * buttonLength, dataWindow, hWnd, 0, hInst, 0);
					
				}
				else
				{
					MessageBoxW(NULL, L"Not enough input!", L"Gas Sorption", MB_OK);
					break;
				}
				break;
			}
			break;

            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
			FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

	

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
		
        break;
    }
    return (INT_PTR)FALSE;
}

//function for calling the Open Dialog Box
wchar_t getFilePath(HWND hWnd, HINSTANCE hInst)
{
	
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
		COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileOpenDialog *pFileOpen;

		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

		if (SUCCEEDED(hr))
		{
				// Show the Open dialog box.
				hr = pFileOpen->Show(NULL);

				// Get the file name from the dialog box.
				if (SUCCEEDED(hr))
				{
					IShellItem *pItem;
					hr = pFileOpen->GetResult(&pItem);
					if (SUCCEEDED(hr))
					{

						PWSTR pszFilePath;
						hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

						// Display the file name to the user.
						if (SUCCEEDED(hr))
						{
							filePath = pszFilePath;
							/*static_out = CreateWindow(L"Static",
								pszFilePath,
								WS_CHILD | WS_VISIBLE,
								500, 50, 300, 100, hWnd, 0, hInst, 0);
							MessageBoxW(NULL, pszFilePath, L"File Path", MB_OK);*/

							CoTaskMemFree(pszFilePath);
						}
						pItem->Release();
					}
				}
				pFileOpen->Release();
			
		}
		
	}
	CoUninitialize();
	return 0;
}

std::vector<std::vector<std::string> > csvRead(PWSTR filePath)
{
	char filePath_char[1024];
	char DefChar = ' ';
	WideCharToMultiByte(CP_ACP, 0, filePath, -1, filePath_char, 1024, &DefChar, NULL);
	//A std:string  using the char* constructor.
	std::string strFilePath(filePath_char);

	CSVReader reader(strFilePath);
	// Get the data from CSV File
	std::vector<std::vector<std::string> > dataList = reader.getData();

	return dataList;
}

std::vector<std::vector<std::string> > CSVReader::getData()
{
	std::ifstream file(fileName);
	std::vector<std::vector<std::string> > dataList;
	std::string line = "";
	// Iterate through each line and split the content using delimeter
	while (getline(file, line))
	{
		std::vector<std::string> vec;
		boost::algorithm::split(vec, line, boost::is_any_of(delimeter));
		dataList.push_back(vec);
	}
	// Close the File
	file.close();
	return dataList;
}

double classSorptionData::heliumCalibration(classSorptionData sorptionData)
{
	int k = stoi(sorptionData.heliumRaw[0][10]); //flag for the first step

	vector<double> T;
	vector<double>T_;
	vector<double>P1;
	vector<double>P2;
	vector<double>P1_;
	vector<double>P2_;
	vector<double>Z1;
	vector<double>Z2;
	vector<double>Z1_;
	vector<double>Z2_;
	double x;
	double Vvoid = 0;
	vector<int> index_BU;
	vector<int> index_EQ;
	vector<double> sVratio;
	double Vratio = 0;

	vector<double> Tsort;
	double medianT;
	double aveT = 0;
	double aveT_ = 0;

	int offsetFlag = 0;
	double offset = 0.0;

	//if data starts after cells are separated, 
	//we collect data from the first valve switch and offset is P2 at that switch
	if (k == 1)
	{
		for (int i = 0; i < sorptionData.heliumRaw.size(); i++)
		{
			//Fist step, get offset
			if ((k == 1) && (stoi(sorptionData.heliumRaw[i][10]) != k) && (offsetFlag == 0))
			{
				//change flag, pick last value prior to the valve switch
				k = 0;
				T.push_back(stod(sorptionData.heliumRaw[i - 1][0]));
				P1.push_back(stod(sorptionData.heliumRaw[i - 1][4]));
				P2.push_back(stod(sorptionData.heliumRaw[i - 1][5]));
				index_BU.push_back(i - 1);
				offset = stod(sorptionData.heliumRaw[i - 1][5]);
				offsetFlag++;
			}
			//Valve open, end buildup
			else if ((k == 1) && (stoi(sorptionData.heliumRaw[i][10]) != k) && (offsetFlag != 0))
			{
				//change flag, pick last value prior to the valve switch
				k = 0;
				T.push_back(stod(sorptionData.heliumRaw[i - 1][0]));
				P1.push_back(stod(sorptionData.heliumRaw[i - 1][4]));
				P2.push_back(stod(sorptionData.heliumRaw[i - 1][5]));
				index_BU.push_back(i - 1);
			}
			//Valve close, end equilibration
			else if ((k == 0) && (stoi(sorptionData.heliumRaw[i][10]) != k))
			{
				//change flag, pick last value prior to the valve switch
				k = 1;
				T_.push_back(stod(sorptionData.heliumRaw[i - 1][0]));
				P1_.push_back(stod(sorptionData.heliumRaw[i - 1][4]));
				P2_.push_back(stod(sorptionData.heliumRaw[i - 1][5]));
				index_EQ.push_back(i - 1);
			}
		}
	}
	//if data starts with cells connected (still vaccuuming)
	//fist switch is not collected because it is not a part of experiment
	//but offset is the P1 value at that switch (a more correct definition but not always available)
	else
	{
		k = 0;
		for (int i = 0; i < sorptionData.heliumRaw.size(); i++)
		{
			//Fist step, get offset
			if ((stoi(sorptionData.heliumRaw[i][10]) != -1) && (k == 0) && (stoi(sorptionData.heliumRaw[i][10]) != k) && (offsetFlag == 0))
			{
				//change flag, pick last value prior to the valve switch
				k = 1;
				/*T.push_back(stod(sorptionData.heliumRaw[i - 1][0]));
				P1.push_back(stod(sorptionData.heliumRaw[i - 1][4]));
				P2.push_back(stod(sorptionData.heliumRaw[i - 1][5]));
				index_BU.push_back(i - 1);*/
				offset = stod(sorptionData.heliumRaw[i - 1][4]);
				offsetFlag++;
			}
			//Valve open, end buildup
			if ((stoi(sorptionData.heliumRaw[i][10]) != -1) && (k == 1) && (stoi(sorptionData.heliumRaw[i][10]) != k))
			{
				//change flag, pick last value prior to the valve switch
				k = 0;
				T.push_back(stod(sorptionData.heliumRaw[i - 1][0]));
				P1.push_back(stod(sorptionData.heliumRaw[i - 1][4]));
				P2.push_back(stod(sorptionData.heliumRaw[i - 1][5]));
				index_BU.push_back(i - 1);
			}
			//Valve close, end equilibration
			else if ((stoi(sorptionData.heliumRaw[i][10]) != -1) && (k == 0) && (stoi(sorptionData.heliumRaw[i][10]) != k) && (offsetFlag != 0))
			{
				//change flag, pick last value prior to the valve switch
				k = 1;
				T_.push_back(stod(sorptionData.heliumRaw[i - 1][0]));
				P1_.push_back(stod(sorptionData.heliumRaw[i - 1][4]));
				P2_.push_back(stod(sorptionData.heliumRaw[i - 1][5]));
				index_EQ.push_back(i - 1);
			}
		}
	}

	//celcius to K
	for (double& d : T) d += 273.15;
	for (double& d : T_) d += 273.15;

	//correct temperature actifacts
	for (int i = 0; i < sorptionData.heliumRaw.size(); i++)
	{
		Tsort.push_back(stod(sorptionData.heliumRaw[i][0]));
	}
	std::sort(Tsort.begin(), Tsort.end());
	medianT = Tsort[round(Tsort.size() / 2)];
	medianT += 273.15;
	int count = 0;
	for (int i = 0; i < sorptionData.heliumSteps; i++)
	{
		if ((T[i] > medianT*0.98) && (T[i] < medianT*1.02))
		{
			aveT += T[i];
			count++;
		}
	}
	aveT /= count;
	count = 0;
	for (int i = 0; i < sorptionData.heliumSteps; i++)
	{
		if ((T_[i] > medianT*0.98) && (T_[i] < medianT*1.02))
		{
			aveT_ += T_[i];
			count++;
		}
	}
	aveT_ /= count;


	//offset pressure
	for (double& d : P1) d -= offset;
	for (double& d : P1_) d -= offset;
	for (double& d : P2) d -= offset;
	for (double& d : P2_) d -= offset;

	//calculating real gas factor
	for (int i = 0; i < sorptionData.heliumSteps; i++)
	{
		Z1.push_back(sorptionData.Z_VdW_He(medianT, P1[i]));
		Z2.push_back(sorptionData.Z_VdW_He(medianT, P2[i]));
		Z1_.push_back(sorptionData.Z_VdW_He(medianT, P1_[i]));
		Z2_.push_back(sorptionData.Z_VdW_He(medianT, P2_[i]));
	}

	//calculating volume ratios
	for (int i = 1; i < sorptionData.heliumSteps; i++) // not using the first step, because pressure is out of range most of the times(< 10bar)
	{
		double a = P1[i] / Z1[i] / medianT;
		double b = P1_[i] / Z1_[i] / medianT;
		double c = b;
		double d = P1_[i - 1] / Z1_[i - 1] / medianT;
		sVratio.push_back((a - b) / (c - d));
		Vratio += (a - b) / (c - d);
	}

	Vratio /= (sorptionData.heliumSteps - 1);
	Vvoid = Vratio * sorptionData.Vrc;

	return Vvoid;
}

vector<vector<double>> classSorptionData::gasSorption(classSorptionData sorptionData)
{
	int k = stoi(sorptionData.gasRaw[0][10]); //flag for the first step

	vector<double> T;
	vector<double>T_;
	vector<double>P1;
	vector<double>P2;
	vector<double>P1_;
	vector<double>P2_;
	vector<double>Z1;
	vector<double>Z2;
	vector<double>Z1_;
	vector<double>Z2_;
	double x;
	double Vvoid = 0;
	vector<int> index_BU;
	vector<int> index_EQ;
	vector<double> sVratio;
	double Vratio = 0;

	vector<double> Tsort;
	double medianT;
	double aveT = 0;
	double aveT_ = 0;

	int offsetFlag = 0;
	double offset = 0.0;

	vector<vector<double>> isotherm;

	//if data starts after cells are separated, 
	//we collect data from the first valve switch and offset is P2 at that switch
	if (k == 1)
	{
		for (int i = 0; i < sorptionData.gasRaw.size(); i++)
		{
			//Fist step, get offset
			if ((k == 1) && (stoi(sorptionData.gasRaw[i][10]) != k) && (offsetFlag == 0))
			{
				//change flag, pick last value prior to the valve switch
				k = 0;
				T.push_back(stod(sorptionData.gasRaw[i - 1][0]));
				P1.push_back(stod(sorptionData.gasRaw[i - 1][4]));
				P2.push_back(stod(sorptionData.gasRaw[i - 1][5]));
				index_BU.push_back(i - 1);
				offset = stod(sorptionData.gasRaw[i - 1][5]);
				offsetFlag++;
			}
			//Valve open, end buildup
			else if ((k == 1) && (stoi(sorptionData.gasRaw[i][10]) != k) && (offsetFlag != 0))
			{
				//change flag, pick last value prior to the valve switch
				k = 0;
				T.push_back(stod(sorptionData.gasRaw[i - 1][0]));
				P1.push_back(stod(sorptionData.gasRaw[i - 1][4]));
				P2.push_back(stod(sorptionData.gasRaw[i - 1][5]));
				index_BU.push_back(i - 1);
			}
			//Valve close, end equilibration
			else if ((k == 0) && (stoi(sorptionData.gasRaw[i][10]) != k))
			{
				//change flag, pick last value prior to the valve switch
				k = 1;
				T_.push_back(stod(sorptionData.gasRaw[i - 1][0]));
				P1_.push_back(stod(sorptionData.gasRaw[i - 1][4]));
				P2_.push_back(stod(sorptionData.gasRaw[i - 1][5]));
				index_EQ.push_back(i - 1);
			}
		}
	}
	//if data starts with cells connected (still vaccuuming)
	//fist switch is not collected because it is not a part of experiment
	//but offset is the P1 value at that switch (a more correct definition but not always available)
	else
	{
		k = 0;
		for (int i = 0; i < sorptionData.gasRaw.size(); i++)
		{
			//Fist step, get offset
			if ((stoi(sorptionData.gasRaw[i][10]) != -1) && (k == 0) && (stoi(sorptionData.gasRaw[i][10]) != k) && (offsetFlag == 0))
			{
				//change flag, pick last value prior to the valve switch
				k = 1;
				/*T.push_back(stod(sorptionData.heliumRaw[i - 1][0]));
				P1.push_back(stod(sorptionData.heliumRaw[i - 1][4]));
				P2.push_back(stod(sorptionData.heliumRaw[i - 1][5]));
				index_BU.push_back(i - 1);*/
				offset = stod(sorptionData.gasRaw[i - 1][4]);
				offsetFlag++;
			}
			//Valve open, end buildup
			if ((stoi(sorptionData.gasRaw[i][10]) != -1) && (k == 1) && (stoi(sorptionData.gasRaw[i][10]) != k))
			{
				//change flag, pick last value prior to the valve switch
				k = 0;
				T.push_back(stod(sorptionData.gasRaw[i - 1][0]));
				P1.push_back(stod(sorptionData.gasRaw[i - 1][4]));
				P2.push_back(stod(sorptionData.gasRaw[i - 1][5]));
				index_BU.push_back(i - 1);
			}
			//Valve close, end equilibration
			else if ((stoi(sorptionData.gasRaw[i][10]) != -1) && (k == 0) && (stoi(sorptionData.gasRaw[i][10]) != k) && (offsetFlag != 0))
			{
				//change flag, pick last value prior to the valve switch
				k = 1;
				T_.push_back(stod(sorptionData.gasRaw[i - 1][0]));
				P1_.push_back(stod(sorptionData.gasRaw[i - 1][4]));
				P2_.push_back(stod(sorptionData.gasRaw[i - 1][5]));
				index_EQ.push_back(i - 1);
			}
		}
	}

	sorptionData.gasSteps = index_EQ.size();

	//celcius to K
	for (double& d : T) d += 273.15;
	for (double& d : T_) d += 273.15;

	//correct temperature actifacts
	for (int i = 0; i < sorptionData.gasRaw.size(); i++)
	{
		Tsort.push_back(stod(sorptionData.gasRaw[i][0]));
	}
	std::sort(Tsort.begin(), Tsort.end());
	medianT = Tsort[round(Tsort.size() / 2)];
	medianT += 273.15;
	int count = 0;
	for (int i = 0; i < sorptionData.gasSteps; i++)
	{
		if ((T[i] > medianT*0.98) && (T[i] < medianT*1.02))
		{
			aveT += T[i];
			count++;
		}
	}
	aveT /= count;
	count = 0;
	for (int i = 0; i < sorptionData.gasSteps; i++)
	{
		if ((T_[i] > medianT*0.98) && (T_[i] < medianT*1.02))
		{
			aveT_ += T_[i];
			count++;
		}
	}
	aveT_ /= count;

	//offset pressure
	for (double& d : P1) d -= offset;
	for (double& d : P1_) d -= offset;
	for (double& d : P2) d -= offset;
	for (double& d : P2_) d -= offset;

	//calculating real gas factor
	for (int i = 0; i < sorptionData.gasSteps; i++)
	{
		Z1.push_back(sorptionData.Z_PR_CH4(aveT, P1[i] / 10, sorptionData));
		//Z2.push_back(sorptionData.Z_PR_CH4(aveT, P2[i]/10, sorptionData));
		Z1_.push_back(sorptionData.Z_PR_CH4(aveT_, P1_[i] / 10, sorptionData));
		//Z2_.push_back(sorptionData.Z_PR_CH4(aveT_, P2_[i]/10, sorptionData));
	}

	//exess sorption calculation
	isotherm.push_back({ 0.0, 0.0 });
	double M_transf = 0;
	//vector<double> M_transf_cum;
	double M_nonS;
	double M_excess;
	for (int i = 0; i < sorptionData.gasSteps; i++)
	{
		M_transf = M_transf + 0.1*sorptionData.Vrc*sorptionData.MCH4 / sorptionData.R * (P1[i] / (Z1[i] * aveT) - P1_[i] / (Z1_[i] * aveT_));
		//M_transf_cum.push_back(M_transf);
		M_nonS = 0.1*P1_[i] * sorptionData.Vvoid*sorptionData.MCH4 / (Z1_[i] * sorptionData.R*aveT_);
		M_excess = 1000 * (M_transf - M_nonS) / sorptionData.mass / sorptionData.MCH4;
		isotherm.push_back({ P1_[i], M_excess });
	}

	return isotherm;
}

double classSorptionData::Z_VdW_He(double T, double P)
{
	//T in K, P in bar
	double Z;

	if ((T < 273.15) || (T > 423.15)) return Z = 0;
	else if ((P < 10) || (P > 174)) return Z = 0;
	else
	{

		double a = -52944.5;
		double u_b = 9.570766;
		double R = 82.0617;
		double u_F = 1.0;
		double Pb = P / 1.01325;

		double v;
		double p2;

		for (int i = 0; i < 10; i++)
		{
			v = 22414 / 273.15*T / Pb * u_F;
			p2 = R * T / (v - u_b) - a / (v*v);
			u_F = p2 * v / (R*T);
		}
		Z = u_F;
		return Z;
	}
}

double classSorptionData::Z_PR_CH4(double T, double P, classSorptionData sorptionData) //Peng-Robinson equiation of state, methane
{
	P *= 1000000; //MPa to Pa
	double R = 8.31415;

	double Tc = 190.564;
	double Pc = 4599200;
	double omega = 0.01142;

	double f_omega = 0.37464 + 1.54226*omega - 0.26992*pow(omega, 2.0);
	double Tr = T / Tc;
	double a = 0.45724 * pow(R, 2.0) * pow(Tc, 2.0) / Pc * pow((1 + f_omega * (1 - pow(Tr, 0.5))), 2);
	double b = 0.0778 *R * Tc / Pc;

	double Af = a * P / pow(R, 2.0) / pow(T, 2.0);
	double Bf = b * P / R / T;

	double v1 = 1.0;
	double v2 = -(1 - Bf);
	double v3 = (Af - 3 * pow(Bf, 2.0) - 2 * Bf);
	double v4 = -(Af*Bf - pow(Bf, 2.0) - pow(Bf, 3.0));

	double Z_PR_CH4 = sorptionData.cardano_min(v1, v2, v3, v4);

	return Z_PR_CH4;

}

double classSorptionData::cardano_min(double a, double b, double c, double d) //cubic equiation solution
{
	double Min;

	double pi = 3.1415926;
	double p = (3 * a*c - pow(b, 2.0)) / 3 / pow(a, 2.0);
	double q = 2 * pow(b, 3.0) / 27 / pow(a, 3.0) - b * c / 3 / pow(a, 2.0) + d / a;
	double discrim = pow((q / 2), 2.0) + pow((p / 3), 3.0);

	if (discrim < 0)
	{
		double phi = acos(-q / 2 / sqrt(pow((-p / 3), 3.0)));
		double y1 = sqrt(-p / 3)*cos(phi / 3);
		double y2 = sqrt(-p / 3)*cos(phi / 3 + 2 * pi / 3);
		double y3 = sqrt(-p / 3)*cos(phi / 3 + 4 * pi / 3);

		double x1 = y1 - b / 3 / a;
		Min = x1;
		double x2 = y2 - b / 3 / a;

		if (x2 < x1)
		{
			Min = x2;
		}

		double x3 = y3 - b / 3 / a;

		if (x3 < x2)
		{
			Min = x3;
		}
	}
	else if (discrim > 0)
	{
		double u = pow((-q / 2 + sqrt(discrim)), 1.0 / 3.0);
		double v = pow((-q / 2 - sqrt(discrim)), 1.0 / 3.0);

		double y1 = u + v;
		double x1 = y1 - b / 3 / a;
		Min = x1;
	}
	else
	{
		double y1 = pow(-4 * q, 1.0 / 3.0);
		double y2 = pow(q / 2, 1.0 / 3.0);

		double x1 = y1 - b / 3 / a;
		Min = x1;
		double x2 = y2 - b / 3 / a;

		if (x2 < x1)
		{
			Min = x2;
		}
	}

	return Min;
}

string getSavePathWithDialog()
{
	string path = "";
	HRESULT hr = CoInitializeEx(NULL, COINITBASE_MULTITHREADED |
		COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileSaveDialog *pFileSave;

		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
			IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave));
		if (SUCCEEDED(hr))
		{
			// Set default extension
			hr = pFileSave->SetDefaultExtension(L"csv");
			if (SUCCEEDED(hr))
			{
				// Show the Open dialog box.
				hr = pFileSave->Show(NULL);

				// Get the file name from the dialog box.
				if (SUCCEEDED(hr))
				{
					IShellItem *pItem;
					hr = pFileSave->GetResult(&pItem);
					if (SUCCEEDED(hr))
					{
						PWSTR pszFilePath;
						hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
						// Display the file name to the user.
						if (SUCCEEDED(hr))
						{
							
							//MessageBox(NULL, pszFilePath, L"File Path", MB_OK);
							char buffer[500];
							wcstombs(buffer, pszFilePath, 500);
							path = buffer;
							CoTaskMemFree(pszFilePath);
						}
						pItem->Release();
					}
				}
				pFileSave->Release();
			}
		}
		CoUninitialize();
	}
	return path;
}

string getLoadPathWithDialog()
{
	string path = "";
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
		COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileOpenDialog *pFileOpen;

		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

		if (SUCCEEDED(hr))
		{
			// Set default extension
			hr = pFileOpen->SetDefaultExtension(L"csv");
			if (SUCCEEDED(hr))
			{
				// Show the Open dialog box.
				hr = pFileOpen->Show(NULL);

				// Get the file name from the dialog box.
				if (SUCCEEDED(hr))
				{
					IShellItem *pItem;
					hr = pFileOpen->GetResult(&pItem);
					if (SUCCEEDED(hr))
					{
						PWSTR pszFilePath;
						hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
						// Display the file name to the user.
						if (SUCCEEDED(hr))
						{

							//MessageBox(NULL, pszFilePath, L"File Path", MB_OK);
							char buffer[500];
							wcstombs(buffer, pszFilePath, 500);
							path = buffer;
							CoTaskMemFree(pszFilePath);
						}
						pItem->Release();
					}
				}
				pFileOpen->Release();
			}
		}

	}
	CoUninitialize();
	return path;
}

void csvWrite(string savePath, classSorptionData sorptionData)
{
	ofstream results;
	results.open(savePath);

	//Header
	results << "Pressure,Excess Sorption\n";

	//isotherm
	for (int i = 0; i < sorptionData.isotherm.size(); i++)
	{
		results << sorptionData.isotherm[i][0] << "," << sorptionData.isotherm[i][1] << "\n";
	}
	results.close();
}

bool is_number(const std::string& s)
{
	char* end = nullptr;
	double val = strtod(s.c_str(), &end);
	return end != s.c_str() && *end == '\0' && val != HUGE_VAL;
}
