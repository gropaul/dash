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
#include "file_next_static_css_7f049ec031a58df3_css.hpp"
#include "file_next_static_css_80b80e7b71bd6f00_css.hpp"
#include "file_next_static_css_fa086eb031a44374_css.hpp"
#include "file_next_static_chunks_762_ff6841768728ab74_js.hpp"
#include "file_next_static_chunks_b12a5a30_3981a25d0c301039_js.hpp"
#include "file_next_static_chunks_webpack_1aaae066fafd081c_js.hpp"
#include "file_next_static_chunks_791_2e95b20546e23eda_js.hpp"
#include "file_next_static_chunks_983_05e5b622d4362471_js.hpp"
#include "file_next_static_chunks_framework_f238c7b8f578dd52_js.hpp"
#include "file_next_static_chunks_273acdc0_964285e9e20397c9_js.hpp"
#include "file_next_static_chunks_909_fe24ba843970cae5_js.hpp"
#include "file_next_static_chunks_754_d89a910eabbfc701_js.hpp"
#include "file_next_static_chunks_0b42ce73_3de0ac7a79eebc02_js.hpp"
#include "file_next_static_chunks_857_f9164c64ae036bf8_js.hpp"
#include "file_next_static_chunks_main_app_6b3fe5afb05c7730_js.hpp"
#include "file_next_static_chunks_dc10eadf_a78d79955ec47c3e_js.hpp"
#include "file_next_static_chunks_272_d4ff4ae2af9386b2_js.hpp"
#include "file_next_static_chunks_93bfa88f_c00912b8f5be66ba_js.hpp"
#include "file_next_static_chunks_6bb80806_352d40863798c5f7_js.hpp"
#include "file_next_static_chunks_polyfills_42372ed130431b0a_js.hpp"
#include "file_next_static_chunks_main_38f8f01dfed9a862_js.hpp"
#include "file_next_static_chunks_app_layout_0bb92367f121d620_js.hpp"
#include "file_next_static_chunks_app_loading_feaed24eaf7480ee_js.hpp"
#include "file_next_static_chunks_app_page_3a58bec987df7cfe_js.hpp"
#include "file_next_static_chunks_app_not_found_page_7b3a5c3b882fff0a_js.hpp"
#include "file_next_static_chunks_pages_error_944a43e45b331146_js.hpp"
#include "file_next_static_chunks_pages_app_be15b270a70492dc_js.hpp"
#include "file_next_static_K2fDoJHh_PDZQZVrKuAj1_ssgManifest_js.hpp"
#include "file_next_static_K2fDoJHh_PDZQZVrKuAj1_buildManifest_js.hpp"
#include "file_next_static_media_d1773e83ecd9f5ec_s_woff2.hpp"
#include "file_next_static_media_0b42914f8036f313_s_p_woff2.hpp"
#include "file_favicon_web_app_manifest_192x192_png.hpp"
#include "file_favicon_web_app_manifest_512x512_png.hpp"
#include "file_fonts_Urbanist_VariableFont_wght_ttf.hpp"

#include <map>

namespace duckdb { 
std::vector<File> files = {
	FILE_ICON_PNG,FILE_FAVICON_ICO,FILE_INDEX_HTML,FILE_APPLE_ICON_PNG,FILE_INDEX_TXT,FILE_404_HTML,FILE_ICON_SVG,FILE_MANIFEST_JSON,FILE_NEXT_STATIC_CSS_7F049EC031A58DF3_CSS,FILE_NEXT_STATIC_CSS_80B80E7B71BD6F00_CSS,FILE_NEXT_STATIC_CSS_FA086EB031A44374_CSS,FILE_NEXT_STATIC_CHUNKS_762_FF6841768728AB74_JS,FILE_NEXT_STATIC_CHUNKS_B12A5A30_3981A25D0C301039_JS,FILE_NEXT_STATIC_CHUNKS_WEBPACK_1AAAE066FAFD081C_JS,FILE_NEXT_STATIC_CHUNKS_791_2E95B20546E23EDA_JS,FILE_NEXT_STATIC_CHUNKS_983_05E5B622D4362471_JS,FILE_NEXT_STATIC_CHUNKS_FRAMEWORK_F238C7B8F578DD52_JS,FILE_NEXT_STATIC_CHUNKS_273ACDC0_964285E9E20397C9_JS,FILE_NEXT_STATIC_CHUNKS_909_FE24BA843970CAE5_JS,FILE_NEXT_STATIC_CHUNKS_754_D89A910EABBFC701_JS,FILE_NEXT_STATIC_CHUNKS_0B42CE73_3DE0AC7A79EEBC02_JS,FILE_NEXT_STATIC_CHUNKS_857_F9164C64AE036BF8_JS,FILE_NEXT_STATIC_CHUNKS_MAIN_APP_6B3FE5AFB05C7730_JS,FILE_NEXT_STATIC_CHUNKS_DC10EADF_A78D79955EC47C3E_JS,FILE_NEXT_STATIC_CHUNKS_272_D4FF4AE2AF9386B2_JS,FILE_NEXT_STATIC_CHUNKS_93BFA88F_C00912B8F5BE66BA_JS,FILE_NEXT_STATIC_CHUNKS_6BB80806_352D40863798C5F7_JS,FILE_NEXT_STATIC_CHUNKS_POLYFILLS_42372ED130431B0A_JS,FILE_NEXT_STATIC_CHUNKS_MAIN_38F8F01DFED9A862_JS,FILE_NEXT_STATIC_CHUNKS_APP_LAYOUT_0BB92367F121D620_JS,FILE_NEXT_STATIC_CHUNKS_APP_LOADING_FEAED24EAF7480EE_JS,FILE_NEXT_STATIC_CHUNKS_APP_PAGE_3A58BEC987DF7CFE_JS,FILE_NEXT_STATIC_CHUNKS_APP_NOT_FOUND_PAGE_7B3A5C3B882FFF0A_JS,FILE_NEXT_STATIC_CHUNKS_PAGES_ERROR_944A43E45B331146_JS,FILE_NEXT_STATIC_CHUNKS_PAGES_APP_BE15B270A70492DC_JS,FILE_NEXT_STATIC_K2FDOJHH_PDZQZVRKUAJ1_SSGMANIFEST_JS,FILE_NEXT_STATIC_K2FDOJHH_PDZQZVRKUAJ1_BUILDMANIFEST_JS,FILE_NEXT_STATIC_MEDIA_D1773E83ECD9F5EC_S_WOFF2,FILE_NEXT_STATIC_MEDIA_0B42914F8036F313_S_P_WOFF2,FILE_FAVICON_WEB_APP_MANIFEST_192X192_PNG,FILE_FAVICON_WEB_APP_MANIFEST_512X512_PNG,FILE_FONTS_URBANIST_VARIABLEFONT_WGHT_TTF
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
