#include "html_frame.hpp"
#include "ie_setup.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace momo
{
	namespace
	{
		void* g_original_func{};
		GUID g_browser_emulation_guid{
			0xac969931, 0x3566, 0x4b50, {0xae, 0x48, 0x71, 0xb9, 0x6a, 0x75, 0xc8, 0x79}
		};

		std::string read_file(const std::filesystem::path& filename)
		{
			std::ifstream ifs(filename, std::ios::binary);
			return {std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};
		}

		bool is_running_in_wine()
		{
			auto* ntdll = GetModuleHandleA("ntdll.dll");
			return ntdll && GetProcAddress(ntdll, "wine_get_version");
		}

		PIMAGE_NT_HEADERS get_nt_headers(const HMODULE module)
		{
			if (!module)
			{
				return nullptr;
			}

			auto* const ptr = reinterpret_cast<uint8_t*>(module);
			const auto* dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(ptr);
			return reinterpret_cast<PIMAGE_NT_HEADERS>(ptr + dos_header->e_lfanew);
		}

		PIMAGE_OPTIONAL_HEADER get_optional_header(const HMODULE module)
		{
			auto* const nt_headers = get_nt_headers(module);
			if (!nt_headers)
			{
				return nullptr;
			}

			return &nt_headers->OptionalHeader;
		}

		std::vector<PIMAGE_SECTION_HEADER> get_section_headers(const HMODULE module)
		{
			std::vector<PIMAGE_SECTION_HEADER> headers{};

			auto* const nt_headers = get_nt_headers(module);
			if (!nt_headers)
			{
				return headers;
			}

			auto section = IMAGE_FIRST_SECTION(nt_headers);
			for (uint16_t i = 0; i < nt_headers->FileHeader.NumberOfSections; ++i, ++section)
			{
				if (section)
				{
					headers.push_back(section);
				}
			}

			return headers;
		}

		void** find_iat_entry(PIMAGE_IMPORT_DESCRIPTOR import_descriptor, uint8_t* module_ptr,
		                      const std::string& target_module_name, FARPROC target_function,
		                      const HMODULE other_module)
		{
			for (; import_descriptor->Name; ++import_descriptor)
			{
				if (target_module_name != reinterpret_cast<char*>(module_ptr + import_descriptor->Name))
				{
					continue;
				}

				auto* original_thunk_data =
					reinterpret_cast<PIMAGE_THUNK_DATA>(import_descriptor->OriginalFirstThunk + module_ptr);
				auto* thunk_data =
					reinterpret_cast<PIMAGE_THUNK_DATA>(import_descriptor->FirstThunk + module_ptr);

				for (; original_thunk_data->u1.AddressOfData; ++original_thunk_data, ++thunk_data)
				{
					if (thunk_data->u1.Function == reinterpret_cast<uint64_t>(target_function))
					{
						return reinterpret_cast<void**>(&thunk_data->u1.Function);
					}

					const size_t ordinal_number = original_thunk_data->u1.AddressOfData & 0xFFFFFFF;
					if (ordinal_number > 0xFFFF)
					{
						continue;
					}

					auto* proc = GetProcAddress(other_module, MAKEINTRESOURCEA(ordinal_number));
					if (proc == target_function)
					{
						return reinterpret_cast<void**>(&thunk_data->u1.Function);
					}
				}
			}

			return nullptr;
		}

		void** get_iat_entry(const HMODULE module, const std::string& target_module_name, const char* proc_name)
		{
			if (!module)
			{
				return nullptr;
			}

			const auto other_module = GetModuleHandleA(target_module_name.data());
			if (!other_module)
			{
				return nullptr;
			}

			auto* const target_function = GetProcAddress(other_module, proc_name);
			if (!target_function)
				return nullptr;

			const auto* header = get_optional_header(module);
			if (!header)
			{
				return nullptr;
			}

			const auto module_ptr = reinterpret_cast<uint8_t*>(module);
			auto* import_descriptor = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(
				module_ptr + header->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

			return find_iat_entry(import_descriptor, module_ptr, target_module_name, target_function, other_module);
		}

		int WINAPI co_internet_feature_value_internal_stub(const GUID* guid, uint32_t* result)
		{
			const auto res =
				reinterpret_cast<decltype(co_internet_feature_value_internal_stub)*>(g_original_func)(guid, result);

			if (IsEqualGUID(*guid, g_browser_emulation_guid))
			{
				*result = 11000;
				return 0;
			}

			return res;
		}

		std::wstring get_module_name(const HMODULE mod)
		{
			std::wstring path;
			path.resize(MAX_PATH);

			const auto size = GetModuleFileNameW(mod, path.data(), static_cast<DWORD>(path.size()));
			path.resize(size);

			return path;
		}

		size_t translate_file_offset_to_rva(const HMODULE file_lib, const size_t file_offset)
		{
			const auto sections = get_section_headers(file_lib);
			for (const auto* section : sections)
			{
				if (section->PointerToRawData <= file_offset &&
					section->PointerToRawData + section->SizeOfRawData > file_offset)
				{
					const auto section_va = file_offset - section->PointerToRawData;
					return section_va + section->VirtualAddress;
				}
			}

			return 0;
		}

		std::pair<size_t, GUID*> find_url_mon_browser_emulator_guid(const HMODULE mapped_urlmon,
		                                                            const std::string& urlmon_data)
		{
			auto* file_lib = reinterpret_cast<HMODULE>(const_cast<char*>(urlmon_data.data()));

			const auto guid_pos = urlmon_data.find(std::string(
				reinterpret_cast<const char*>(&g_browser_emulation_guid), sizeof(g_browser_emulation_guid)));
			if (guid_pos == std::string::npos)
			{
				return {0, nullptr};
			}

			const auto guid_rva = translate_file_offset_to_rva(file_lib, guid_pos);
			const auto guid_va = reinterpret_cast<GUID*>(reinterpret_cast<uint8_t*>(mapped_urlmon) + guid_rva);

			if (!IsEqualGUID(*guid_va, g_browser_emulation_guid))
			{
				return {0, nullptr};
			}

			return {guid_rva, guid_va};
		}

		size_t find_url_mon_browser_emulator_guid_pointer_rva(const std::string& urlmon_data, const size_t guid_rva)
		{
			auto* file_lib = reinterpret_cast<HMODULE>(const_cast<char*>(urlmon_data.data()));

			const size_t unrelocated_guid_va = get_optional_header(file_lib)->ImageBase + guid_rva;
			const auto guid_ptr_pos = urlmon_data.find(
				std::string(reinterpret_cast<const char*>(&unrelocated_guid_va), sizeof(unrelocated_guid_va)));
			if (guid_ptr_pos == std::string::npos)
			{
				return 0;
			}

			return translate_file_offset_to_rva(file_lib, guid_ptr_pos);
		}

		void patch_cached_browser_emulator(const HMODULE urlmon)
		{
			const auto urlmon_data = read_file(get_module_name(urlmon));
			if (urlmon_data.empty())
			{
				return;
			}

			const auto [guid_rva, guid_va] = find_url_mon_browser_emulator_guid(urlmon, urlmon_data);

			if (!guid_va)
			{
				return;
			}

			const auto guidPtrRVA = find_url_mon_browser_emulator_guid_pointer_rva(urlmon_data, guid_rva);

			if (guidPtrRVA)
			{
				*reinterpret_cast<GUID**>(reinterpret_cast<uint8_t*>(urlmon) + guidPtrRVA) = guid_va;
			}
		}

		void setup_ie_hooks()
		{
			if (is_running_in_wine())
			{
				return;
			}

			const auto urlmon = LoadLibraryA("urlmon.dll");
			const auto target = get_iat_entry(urlmon, "iertutil.dll", MAKEINTRESOURCEA(700));

			DWORD old_protect{};
			VirtualProtect(target, sizeof(target), PAGE_READWRITE, &old_protect);

			g_original_func = *target;
			*target = reinterpret_cast<void*>(&co_internet_feature_value_internal_stub);

			VirtualProtect(target, sizeof(target), old_protect, &old_protect);

			patch_cached_browser_emulator(urlmon);
		}

		void setup_ole()
		{
			static struct ole_initializer
			{
				ole_initializer()
				{
					if (OleInitialize(nullptr) != S_OK)
					{
						throw std::runtime_error("Unable to initialize the OLE library");
					}
				}

				~ole_initializer()
				{
					OleUninitialize();
				}
			} init;
			(void)init;
		}

		void enable_dpi_awareness()
		{
			const auto user32 = LoadLibraryA("user32.dll");

			{
				const auto set_dpi = user32
					                     ? reinterpret_cast<BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT)>(
						                     GetProcAddress(user32, "SetProcessDpiAwarenessContext"))
					                     : nullptr;
				if (set_dpi)
				{
					set_dpi(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
					return;
				}
			}

			{
				const auto shcore = LoadLibraryA("shcore.dll");
				const auto set_dpi = shcore
					                     ? reinterpret_cast<HRESULT(WINAPI*)(PROCESS_DPI_AWARENESS)>(
						                     GetProcAddress(shcore, "SetProcessDpiAwareness"))
					                     : nullptr;
				if (set_dpi)
				{
					set_dpi(PROCESS_PER_MONITOR_DPI_AWARE);
					return;
				}
			}

			{
				const auto set_dpi =
					user32
						? reinterpret_cast<BOOL(WINAPI*)()>(GetProcAddress(user32, "SetProcessDPIAware"))
						: nullptr;
				if (set_dpi)
				{
					set_dpi();
				}
			}
		}
	}

	void setup_internet_explorer()
	{
		static const auto _ = []
		{
			enable_dpi_awareness();
			setup_ie_hooks();
			setup_ole();

			return 0;
		}();
		(void)_;
	}
}
