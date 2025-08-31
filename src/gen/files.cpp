#include "files.hpp"

#include "duckdb/common/optional_ptr.hpp"
#include "duckdb/common/string_util.hpp"

#include "file_icon_png.hpp"
#include "file_favicon_ico.hpp"
#include "file_index_html.hpp"
#include "file_apple_icon_png.hpp"
#include "file_index_txt.hpp"
#include "file_404_html.hpp"
#include "file_icon_svg.hpp"
#include "file_manifest_json.hpp"
#include "file_next_static_css_161a49711a1b0509_css.hpp"
#include "file_next_static_css_60b2e10d0c51dc30_css.hpp"
#include "file_next_static_css_5b54331129c9114c_css.hpp"
#include "file_next_static_chunks_273acdc0_cb8b569f7d3749e9_js.hpp"
#include "file_next_static_chunks_93bfa88f_a0ead23c4e6cf31c_js.hpp"
#include "file_next_static_chunks_3c1c9f4a_4225c13068d984af_js.hpp"
#include "file_next_static_chunks_168_3133b9ddc25904be_js.hpp"
#include "file_next_static_chunks_0b42ce73_1c881b8431b170f5_js.hpp"
#include "file_next_static_chunks_75146d7d_7f39bd7aa2b2e138_js.hpp"
#include "file_next_static_chunks_907_4e6df512f6795fee_js.hpp"
#include "file_next_static_chunks_framework_f238c7b8f578dd52_js.hpp"
#include "file_next_static_chunks_9917b064_2399f393a40ad689_js.hpp"
#include "file_next_static_chunks_566_f5be07bac57bfb19_js.hpp"
#include "file_next_static_chunks_main_app_83a00a8fec35b2cc_js.hpp"
#include "file_next_static_chunks_main_e20ebf217b3711cb_js.hpp"
#include "file_next_static_chunks_6bb80806_e6a302ad6e1790c1_js.hpp"
#include "file_next_static_chunks_951_7067eb44ad499754_js.hpp"
#include "file_next_static_chunks_dc10eadf_a3e4bc37fe6da484_js.hpp"
#include "file_next_static_chunks_webpack_965c5e418e3d1c7c_js.hpp"
#include "file_next_static_chunks_32fa3855_d097c4924821aca6_js.hpp"
#include "file_next_static_chunks_491_970d70f26044e53e_js.hpp"
#include "file_next_static_chunks_146_82392d2c06a6d205_js.hpp"
#include "file_next_static_chunks_polyfills_42372ed130431b0a_js.hpp"
#include "file_next_static_chunks_585_1b8a58483f7055aa_js.hpp"
#include "file_next_static_chunks_app_layout_661b53a0604fc2b7_js.hpp"
#include "file_next_static_chunks_app_page_311381137bd3cb51_js.hpp"
#include "file_next_static_chunks_app_not_found_page_b1d433f3fd34da0b_js.hpp"
#include "file_next_static_chunks_pages_error_754a8f8e0233ea38_js.hpp"
#include "file_next_static_chunks_pages_app_87c429e17d5390d1_js.hpp"
#include "file_next_static_S_RU49ApMWeG5_BFhdRgG_ssgManifest_js.hpp"
#include "file_next_static_S_RU49ApMWeG5_BFhdRgG_buildManifest_js.hpp"
#include "file_next_static_media_7323b9d087306adb_s_p_woff2.hpp"
#include "file_next_static_media_959fe13ebd2a94e9_s_woff2.hpp"
#include "file_favicon_web_app_manifest_192x192_png.hpp"
#include "file_favicon_web_app_manifest_512x512_png.hpp"
#include "file_fonts_Urbanist_VariableFont_wght_ttf.hpp"

#include <map>

namespace duckdb { 
std::vector<File> files = {
	FILE_ICON_PNG,FILE_FAVICON_ICO,FILE_INDEX_HTML,FILE_APPLE_ICON_PNG,FILE_INDEX_TXT,FILE_404_HTML,FILE_ICON_SVG,FILE_MANIFEST_JSON,FILE_NEXT_STATIC_CSS_161A49711A1B0509_CSS,FILE_NEXT_STATIC_CSS_60B2E10D0C51DC30_CSS,FILE_NEXT_STATIC_CSS_5B54331129C9114C_CSS,FILE_NEXT_STATIC_CHUNKS_273ACDC0_CB8B569F7D3749E9_JS,FILE_NEXT_STATIC_CHUNKS_93BFA88F_A0EAD23C4E6CF31C_JS,FILE_NEXT_STATIC_CHUNKS_3C1C9F4A_4225C13068D984AF_JS,FILE_NEXT_STATIC_CHUNKS_168_3133B9DDC25904BE_JS,FILE_NEXT_STATIC_CHUNKS_0B42CE73_1C881B8431B170F5_JS,FILE_NEXT_STATIC_CHUNKS_75146D7D_7F39BD7AA2B2E138_JS,FILE_NEXT_STATIC_CHUNKS_907_4E6DF512F6795FEE_JS,FILE_NEXT_STATIC_CHUNKS_FRAMEWORK_F238C7B8F578DD52_JS,FILE_NEXT_STATIC_CHUNKS_9917B064_2399F393A40AD689_JS,FILE_NEXT_STATIC_CHUNKS_566_F5BE07BAC57BFB19_JS,FILE_NEXT_STATIC_CHUNKS_MAIN_APP_83A00A8FEC35B2CC_JS,FILE_NEXT_STATIC_CHUNKS_MAIN_E20EBF217B3711CB_JS,FILE_NEXT_STATIC_CHUNKS_6BB80806_E6A302AD6E1790C1_JS,FILE_NEXT_STATIC_CHUNKS_951_7067EB44AD499754_JS,FILE_NEXT_STATIC_CHUNKS_DC10EADF_A3E4BC37FE6DA484_JS,FILE_NEXT_STATIC_CHUNKS_WEBPACK_965C5E418E3D1C7C_JS,FILE_NEXT_STATIC_CHUNKS_32FA3855_D097C4924821ACA6_JS,FILE_NEXT_STATIC_CHUNKS_491_970D70F26044E53E_JS,FILE_NEXT_STATIC_CHUNKS_146_82392D2C06A6D205_JS,FILE_NEXT_STATIC_CHUNKS_POLYFILLS_42372ED130431B0A_JS,FILE_NEXT_STATIC_CHUNKS_585_1B8A58483F7055AA_JS,FILE_NEXT_STATIC_CHUNKS_APP_LAYOUT_661B53A0604FC2B7_JS,FILE_NEXT_STATIC_CHUNKS_APP_PAGE_311381137BD3CB51_JS,FILE_NEXT_STATIC_CHUNKS_APP_NOT_FOUND_PAGE_B1D433F3FD34DA0B_JS,FILE_NEXT_STATIC_CHUNKS_PAGES_ERROR_754A8F8E0233EA38_JS,FILE_NEXT_STATIC_CHUNKS_PAGES_APP_87C429E17D5390D1_JS,FILE_NEXT_STATIC_S_RU49APMWEG5_BFHDRGG_SSGMANIFEST_JS,FILE_NEXT_STATIC_S_RU49APMWEG5_BFHDRGG_BUILDMANIFEST_JS,FILE_NEXT_STATIC_MEDIA_7323B9D087306ADB_S_P_WOFF2,FILE_NEXT_STATIC_MEDIA_959FE13EBD2A94E9_S_WOFF2,FILE_FAVICON_WEB_APP_MANIFEST_192X192_PNG,FILE_FAVICON_WEB_APP_MANIFEST_512X512_PNG,FILE_FONTS_URBANIST_VARIABLEFONT_WGHT_TTF
};

optional_ptr<File> GetFile(Path path, const bool try_resolve_404) {
	if (path.empty() || path == "/") {
		path = "index.html";
	}

	// Append / before and after
	path = "/" + path + "/";
	path = StringUtil::Replace(path, "//", "/");

	for (auto &file : files) {
		if (file.path == path) {
			return file;
		}
	}

	if (try_resolve_404) {
		GetFile("/404.html/", false);
	}

	return nullptr;
}

}
