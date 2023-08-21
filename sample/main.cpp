#include <html_ui.hpp>

int APIENTRY WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
	html_ui window("Test", 500, 300);

	window.load_html(R"code(
<!DOCTYPE html>
<html>
<body> 

<h1>Heading 1</h1>
<button onclick="window.external.testFunc('hello world', 1234)">Button</button>

</body>
</html>
)code");

	window.register_handler("testFunc", [](const std::string& str, uint64_t val)
	{
			MessageBoxA(0, str.data(), 0, 0);
	});

	window.show();

	return 0;
}
