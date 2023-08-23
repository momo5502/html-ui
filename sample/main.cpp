#include <momo/html_ui.hpp>

void create_window_1(momo::html_ui& window)
{
	window.register_handler("showMessageBox", [](const std::string& title, const std::string& message) -> std::string
	{
		MessageBoxA(nullptr, message.data(), title.data(), MB_ICONINFORMATION);
		return "OK";
	});

	window.register_raw_handler("rawFunction",
	                            [&window](const std::vector<momo::html_value>& arguments) -> momo::html_value
	                            {
		                            window.evaluate(
			                            "window.external.showMessageBox('Hello', '" + std::to_string(arguments.size()) +
			                            " arguments passed')");
		                            return {};
	                            });

	window.load_html(R"code(
<!DOCTYPE html>
<html>

<head>
    <style>
        html {
            height: 100%;

            background: hsla(139, 70%, 75%, 1);
            background: linear-gradient(90deg, hsla(139, 70%, 75%, 1) 0%, hsla(63, 90%, 76%, 1) 100%);
            background: -moz-linear-gradient(90deg, hsla(139, 70%, 75%, 1) 0%, hsla(63, 90%, 76%, 1) 100%);
            background: -webkit-linear-gradient(90deg, hsla(139, 70%, 75%, 1) 0%, hsla(63, 90%, 76%, 1) 100%);
            filter: progid: DXImageTransform.Microsoft.gradient(startColorstr="#95ECB0", endColorstr="#F3F98A", GradientType=1);
        }

        * {
            font-family: -apple-system, BlinkMacSystemFont, segoe ui, Roboto, Oxygen, Ubuntu, Cantarell, open sans, helvetica neue, sans-serif;
        }

        h1 {
            color: rgb(57, 57, 57);
        }

        button {
            margin: 5px;
            display: inline-block;
            outline: none;
            cursor: pointer;
            font-size: 14px;
            line-height: 1;
            border-radius: 500px;
            transition-property: background-color, border-color, color, box-shadow, filter;
            transition-duration: .3s;
            border: 1px solid transparent;
            letter-spacing: 2px;
            min-width: 160px;
            text-transform: uppercase;
            white-space: normal;
            font-weight: 700;
            text-align: center;
            padding: 16px 14px 18px;
            color: rgb(57, 57, 57);
            box-shadow: inset 0 0 0 2px rgb(57, 57, 57);
            background-color: transparent;
            height: 48px;
        }

        button:hover,
        button.active {
            color: #fff;
            background-color: rgb(57, 57, 57);
        }
    </style>
</head>

<body>
    <center>
        <h1>HTML-UI</h1>
        <br>
        <button onclick="window.external.rawFunction('hello world', 1234)" class="active">Button</button>
		<button onclick="window.external.showMessageBox('Test', 'Hello World')">Button 2</button>
    </center>
</body>

</html>
)code");
}


void create_window_2(momo::html_ui& window)
{
	window.register_handler("showMessageBox", [](const std::string& title, const std::string& message)
	{
		MessageBoxA(nullptr, message.data(), title.data(), MB_ICONINFORMATION);
	});

	window.load_html(R"code(
<!DOCTYPE html>
<html>

<head>
    <style>
        html {
            height: 100%;

            background: hsla(280, 95%, 57%, 1);
            background: linear-gradient(90deg, hsla(280, 95%, 57%, 1) 0%, hsla(193, 90%, 55%, 1) 100%);
            background: -moz-linear-gradient(90deg, hsla(280, 95%, 57%, 1) 0%, hsla(193, 90%, 55%, 1) 100%);
            background: -webkit-linear-gradient(90deg, hsla(280, 95%, 57%, 1) 0%, hsla(193, 90%, 55%, 1) 100%);
            filter: progid: DXImageTransform.Microsoft.gradient(startColorstr="#B429F9", endColorstr="#26C5F3", GradientType=1);
        }

        * {
            font-family: -apple-system, BlinkMacSystemFont, segoe ui, Roboto, Oxygen, Ubuntu, Cantarell, open sans, helvetica neue, sans-serif;
        }

        h1 {
            color: rgb(57, 57, 57);
        }

        button {
            background-color: rgba(240, 240, 240, 0.26);
            border: 1px solid #DFDFDF;
            border-radius: 16px;
            color: rgb(39, 39, 39);
            cursor: pointer;
            line-height: 28px;
            padding: 14px 52px;
            text-decoration: none;
            transition: all .2s;
            user-select: none;
            -webkit-user-select: none;
            touch-action: manipulation;
            font-size: 20px;
        }

        button:active,
        button:hover {
            outline: 0;
        }

        button:hover {
            background-color: rgba(255, 255, 255, 0.4);
        }
    </style>
</head>

<body>
    <center>
        <h1>GAME LAUNCHER</h1>
        <br>
        <button onclick="window.external.showMessageBox('Hi', 'hello world')" class="active">Button</button>
    </center>
</body>

</html>
)code");
}

int APIENTRY WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
	momo::html_ui window1("Test", 500, 300);
	momo::html_ui window2("Test2", 500, 300);

	create_window_1(window1);
	create_window_2(window2);

	momo::html_ui::show_windows();

	return 0;
}
