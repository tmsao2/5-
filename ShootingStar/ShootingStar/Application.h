#pragma once
#include "Geometry.h"
#include <memory>

class Dx12Wrapper;

class Application
{
private:
	Application();
	Application(const Application&) = delete;
	void operator=(Application&) = delete;
	bool InitWindow();
	std::shared_ptr<Dx12Wrapper> _dx12;

public:
	~Application();
	bool Init();
	void Run();
	void Terminate();
	Size GetWindowSize();
	static Application& Instance();
};

