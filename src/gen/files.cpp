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
#include "file_next_static_RfPNVvZUUK6JhS2E_njqu_ssgManifest_js.hpp"
#include "file_next_static_RfPNVvZUUK6JhS2E_njqu_buildManifest_js.hpp"
#include "file_next_static_css_7f049ec031a58df3_css.hpp"
#include "file_next_static_css_fa086eb031a44374_css.hpp"
#include "file_next_static_css_bff9be9fdf3bb141_css.hpp"
#include "file_next_static_chunks_192_724823bb269333d4_js.hpp"
#include "file_next_static_chunks_b12a5a30_3981a25d0c301039_js.hpp"
#include "file_next_static_chunks_791_2e95b20546e23eda_js.hpp"
#include "file_next_static_chunks_framework_f238c7b8f578dd52_js.hpp"
#include "file_next_static_chunks_720_3f1bdd6d75d2bdae_js.hpp"
#include "file_next_static_chunks_909_fe24ba843970cae5_js.hpp"
#include "file_next_static_chunks_754_d89a910eabbfc701_js.hpp"
#include "file_next_static_chunks_0b42ce73_3de0ac7a79eebc02_js.hpp"
#include "file_next_static_chunks_541_92ce931deb51db8e_js.hpp"
#include "file_next_static_chunks_main_app_6b3fe5afb05c7730_js.hpp"
#include "file_next_static_chunks_dc10eadf_a78d79955ec47c3e_js.hpp"
#include "file_next_static_chunks_862_30c9c157fc05ff0a_js.hpp"
#include "file_next_static_chunks_webpack_d5f9ec9636609e44_js.hpp"
#include "file_next_static_chunks_520_7bcded7194ddd486_js.hpp"
#include "file_next_static_chunks_93bfa88f_c00912b8f5be66ba_js.hpp"
#include "file_next_static_chunks_6bb80806_352d40863798c5f7_js.hpp"
#include "file_next_static_chunks_polyfills_42372ed130431b0a_js.hpp"
#include "file_next_static_chunks_173_8741f941ffdd5752_js.hpp"
#include "file_next_static_chunks_main_38f8f01dfed9a862_js.hpp"
#include "file_next_static_chunks_app_loading_feaed24eaf7480ee_js.hpp"
#include "file_next_static_chunks_app_layout_88eb661b8991283c_js.hpp"
#include "file_next_static_chunks_app_page_bda349be37ee2d3a_js.hpp"
#include "file_next_static_chunks_app_test_page_3254d4da813687cb_js.hpp"
#include "file_next_static_chunks_app_not_found_page_7b3a5c3b882fff0a_js.hpp"
#include "file_next_static_chunks_pages_error_944a43e45b331146_js.hpp"
#include "file_next_static_chunks_pages_app_be15b270a70492dc_js.hpp"
#include "file_next_static_media_d1773e83ecd9f5ec_s_woff2.hpp"
#include "file_next_static_media_0b42914f8036f313_s_p_woff2.hpp"
#include "file_favicon_web_app_manifest_192x192_png.hpp"
#include "file_favicon_web_app_manifest_512x512_png.hpp"
#include "file_fonts_Urbanist_VariableFont_wght_ttf.hpp"

#include <map>

namespace duckdb { 
std::vector<File> files = {
	FILE_ICON_PNG,FILE_FAVICON_ICO,FILE_INDEX_HTML,FILE_APPLE_ICON_PNG,FILE_INDEX_TXT,FILE_404_HTML,FILE_ICON_SVG,FILE_TEST_HTML,FILE_MANIFEST_JSON,FILE_TEST_TXT,FILE_NEXT_STATIC_RFPNVVZUUK6JHS2E_NJQU_SSGMANIFEST_JS,FILE_NEXT_STATIC_RFPNVVZUUK6JHS2E_NJQU_BUILDMANIFEST_JS,FILE_NEXT_STATIC_CSS_7F049EC031A58DF3_CSS,FILE_NEXT_STATIC_CSS_FA086EB031A44374_CSS,FILE_NEXT_STATIC_CSS_BFF9BE9FDF3BB141_CSS,FILE_NEXT_STATIC_CHUNKS_192_724823BB269333D4_JS,FILE_NEXT_STATIC_CHUNKS_B12A5A30_3981A25D0C301039_JS,FILE_NEXT_STATIC_CHUNKS_791_2E95B20546E23EDA_JS,FILE_NEXT_STATIC_CHUNKS_FRAMEWORK_F238C7B8F578DD52_JS,FILE_NEXT_STATIC_CHUNKS_720_3F1BDD6D75D2BDAE_JS,FILE_NEXT_STATIC_CHUNKS_909_FE24BA843970CAE5_JS,FILE_NEXT_STATIC_CHUNKS_754_D89A910EABBFC701_JS,FILE_NEXT_STATIC_CHUNKS_0B42CE73_3DE0AC7A79EEBC02_JS,FILE_NEXT_STATIC_CHUNKS_541_92CE931DEB51DB8E_JS,FILE_NEXT_STATIC_CHUNKS_MAIN_APP_6B3FE5AFB05C7730_JS,FILE_NEXT_STATIC_CHUNKS_DC10EADF_A78D79955EC47C3E_JS,FILE_NEXT_STATIC_CHUNKS_862_30C9C157FC05FF0A_JS,FILE_NEXT_STATIC_CHUNKS_WEBPACK_D5F9EC9636609E44_JS,FILE_NEXT_STATIC_CHUNKS_520_7BCDED7194DDD486_JS,FILE_NEXT_STATIC_CHUNKS_93BFA88F_C00912B8F5BE66BA_JS,FILE_NEXT_STATIC_CHUNKS_6BB80806_352D40863798C5F7_JS,FILE_NEXT_STATIC_CHUNKS_POLYFILLS_42372ED130431B0A_JS,FILE_NEXT_STATIC_CHUNKS_173_8741F941FFDD5752_JS,FILE_NEXT_STATIC_CHUNKS_MAIN_38F8F01DFED9A862_JS,FILE_NEXT_STATIC_CHUNKS_APP_LOADING_FEAED24EAF7480EE_JS,FILE_NEXT_STATIC_CHUNKS_APP_LAYOUT_88EB661B8991283C_JS,FILE_NEXT_STATIC_CHUNKS_APP_PAGE_BDA349BE37EE2D3A_JS,FILE_NEXT_STATIC_CHUNKS_APP_TEST_PAGE_3254D4DA813687CB_JS,FILE_NEXT_STATIC_CHUNKS_APP_NOT_FOUND_PAGE_7B3A5C3B882FFF0A_JS,FILE_NEXT_STATIC_CHUNKS_PAGES_ERROR_944A43E45B331146_JS,FILE_NEXT_STATIC_CHUNKS_PAGES_APP_BE15B270A70492DC_JS,FILE_NEXT_STATIC_MEDIA_D1773E83ECD9F5EC_S_WOFF2,FILE_NEXT_STATIC_MEDIA_0B42914F8036F313_S_P_WOFF2,FILE_FAVICON_WEB_APP_MANIFEST_192X192_PNG,FILE_FAVICON_WEB_APP_MANIFEST_512X512_PNG,FILE_FONTS_URBANIST_VARIABLEFONT_WGHT_TTF
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
