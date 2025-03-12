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
#include "file_test_html.hpp"
#include "file_manifest_json.hpp"
#include "file_test_txt.hpp"
#include "file_next_static_css_725acac972cb2d86_css.hpp"
#include "file_next_static_css_7f049ec031a58df3_css.hpp"
#include "file_next_static_css_06037a4dc34b6415_css.hpp"
#include "file_next_static_chunks_main_17dfdc027c05ed9d_js.hpp"
#include "file_next_static_chunks_217_171c38286374368a_js.hpp"
#include "file_next_static_chunks_6bb80806_1baed5c03ff54307_js.hpp"
#include "file_next_static_chunks_main_app_22d963ef3fe53e14_js.hpp"
#include "file_next_static_chunks_0b42ce73_468184a4882d6ce8_js.hpp"
#include "file_next_static_chunks_313_b5abe9683aa28e72_js.hpp"
#include "file_next_static_chunks_88_2f7fd6895021e4f4_js.hpp"
#include "file_next_static_chunks_559_2d5e7d4007d79fa2_js.hpp"
#include "file_next_static_chunks_93bfa88f_6ee39640c9fe86a0_js.hpp"
#include "file_next_static_chunks_736_9bce7fde2afcc044_js.hpp"
#include "file_next_static_chunks_framework_ded83d71b51ce901_js.hpp"
#include "file_next_static_chunks_493ab29e_5a13789b21fa6218_js.hpp"
#include "file_next_static_chunks_764_903eb5e7f2b2f294_js.hpp"
#include "file_next_static_chunks_301_a5b86f86b4182339_js.hpp"
#include "file_next_static_chunks_c78ac0aa_3ace17cad8c90898_js.hpp"
#include "file_next_static_chunks_webpack_f55689b1fccb6653_js.hpp"
#include "file_next_static_chunks_polyfills_78c92fac7aa8fdd8_js.hpp"
#include "file_next_static_chunks_584_56872ac185de22fc_js.hpp"
#include "file_next_static_chunks_240_cde431422ad46ab8_js.hpp"
#include "file_next_static_chunks_app_page_31f3d8757b8f19a8_js.hpp"
#include "file_next_static_chunks_app_layout_301fb5dbb33f4b3d_js.hpp"
#include "file_next_static_chunks_app_loading_8f435599db3e2af4_js.hpp"
#include "file_next_static_chunks_app_test_page_a2c730b97fdf71d5_js.hpp"
#include "file_next_static_chunks_app_not_found_page_5fae1d7300910d3e_js.hpp"
#include "file_next_static_chunks_pages_error_f251a2e1c1b3c71c_js.hpp"
#include "file_next_static_chunks_pages_app_5db5e40c01741300_js.hpp"
#include "file_next_static_nuNWry37fA7uxJMC00nmG_ssgManifest_js.hpp"
#include "file_next_static_nuNWry37fA7uxJMC00nmG_buildManifest_js.hpp"
#include "file_next_static_media_d1773e83ecd9f5ec_s_woff2.hpp"
#include "file_next_static_media_0b42914f8036f313_s_p_woff2.hpp"
#include "file_favicon_web_app_manifest_192x192_png.hpp"
#include "file_favicon_web_app_manifest_512x512_png.hpp"
#include "file_fonts_Urbanist_VariableFont_wght_ttf.hpp"

#include <map>

namespace duckdb { 
std::vector<File> files = {
	FILE_ICON_PNG,FILE_FAVICON_ICO,FILE_INDEX_HTML,FILE_APPLE_ICON_PNG,FILE_INDEX_TXT,FILE_404_HTML,FILE_ICON_SVG,FILE_TEST_HTML,FILE_MANIFEST_JSON,FILE_TEST_TXT,FILE_NEXT_STATIC_CSS_725ACAC972CB2D86_CSS,FILE_NEXT_STATIC_CSS_7F049EC031A58DF3_CSS,FILE_NEXT_STATIC_CSS_06037A4DC34B6415_CSS,FILE_NEXT_STATIC_CHUNKS_MAIN_17DFDC027C05ED9D_JS,FILE_NEXT_STATIC_CHUNKS_217_171C38286374368A_JS,FILE_NEXT_STATIC_CHUNKS_6BB80806_1BAED5C03FF54307_JS,FILE_NEXT_STATIC_CHUNKS_MAIN_APP_22D963EF3FE53E14_JS,FILE_NEXT_STATIC_CHUNKS_0B42CE73_468184A4882D6CE8_JS,FILE_NEXT_STATIC_CHUNKS_313_B5ABE9683AA28E72_JS,FILE_NEXT_STATIC_CHUNKS_88_2F7FD6895021E4F4_JS,FILE_NEXT_STATIC_CHUNKS_559_2D5E7D4007D79FA2_JS,FILE_NEXT_STATIC_CHUNKS_93BFA88F_6EE39640C9FE86A0_JS,FILE_NEXT_STATIC_CHUNKS_736_9BCE7FDE2AFCC044_JS,FILE_NEXT_STATIC_CHUNKS_FRAMEWORK_DED83D71B51CE901_JS,FILE_NEXT_STATIC_CHUNKS_493AB29E_5A13789B21FA6218_JS,FILE_NEXT_STATIC_CHUNKS_764_903EB5E7F2B2F294_JS,FILE_NEXT_STATIC_CHUNKS_301_A5B86F86B4182339_JS,FILE_NEXT_STATIC_CHUNKS_C78AC0AA_3ACE17CAD8C90898_JS,FILE_NEXT_STATIC_CHUNKS_WEBPACK_F55689B1FCCB6653_JS,FILE_NEXT_STATIC_CHUNKS_POLYFILLS_78C92FAC7AA8FDD8_JS,FILE_NEXT_STATIC_CHUNKS_584_56872AC185DE22FC_JS,FILE_NEXT_STATIC_CHUNKS_240_CDE431422AD46AB8_JS,FILE_NEXT_STATIC_CHUNKS_APP_PAGE_31F3D8757B8F19A8_JS,FILE_NEXT_STATIC_CHUNKS_APP_LAYOUT_301FB5DBB33F4B3D_JS,FILE_NEXT_STATIC_CHUNKS_APP_LOADING_8F435599DB3E2AF4_JS,FILE_NEXT_STATIC_CHUNKS_APP_TEST_PAGE_A2C730B97FDF71D5_JS,FILE_NEXT_STATIC_CHUNKS_APP_NOT_FOUND_PAGE_5FAE1D7300910D3E_JS,FILE_NEXT_STATIC_CHUNKS_PAGES_ERROR_F251A2E1C1B3C71C_JS,FILE_NEXT_STATIC_CHUNKS_PAGES_APP_5DB5E40C01741300_JS,FILE_NEXT_STATIC_NUNWRY37FA7UXJMC00NMG_SSGMANIFEST_JS,FILE_NEXT_STATIC_NUNWRY37FA7UXJMC00NMG_BUILDMANIFEST_JS,FILE_NEXT_STATIC_MEDIA_D1773E83ECD9F5EC_S_WOFF2,FILE_NEXT_STATIC_MEDIA_0B42914F8036F313_S_P_WOFF2,FILE_FAVICON_WEB_APP_MANIFEST_192X192_PNG,FILE_FAVICON_WEB_APP_MANIFEST_512X512_PNG,FILE_FONTS_URBANIST_VARIABLEFONT_WGHT_TTF
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
