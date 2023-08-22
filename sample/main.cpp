#include <momo/html_ui.hpp>

int APIENTRY WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
	momo::html_ui window("Test", 500, 300);

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

        button:hover {
            color: #fff;
            background-color: rgb(57, 57, 57);
        }
    </style>
</head>

<body>
    <center>
        <h1>HTML-UI</h1>
        <br>
        <button onclick="window.external.testFunc('hello world', 1234)">Button</button>
    </center>
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
