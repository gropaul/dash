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
#include "file_next_static_css_60b2e10d0c51dc30_css.hpp"
#include "file_next_static_css_21bb8425bd7146cc_css.hpp"
#include "file_next_static_css_5b54331129c9114c_css.hpp"
#include "file_next_static_chunks_75146d7d_09aa1a27ef8ffd85_js.hpp"
#include "file_next_static_chunks_dc10eadf_140341d4bbe49ac0_js.hpp"
#include "file_next_static_chunks_273acdc0_074651c6dcce57aa_js.hpp"
#include "file_next_static_chunks_122_3ec9e1262bd14e23_js.hpp"
#include "file_next_static_chunks_93bfa88f_cc391f3208003935_js.hpp"
#include "file_next_static_chunks_872_6e627e6a293ed396_js.hpp"
#include "file_next_static_chunks_framework_f238c7b8f578dd52_js.hpp"
#include "file_next_static_chunks_0b42ce73_8a46a4463856ecb8_js.hpp"
#include "file_next_static_chunks_32fa3855_229c70a9f91729d1_js.hpp"
#include "file_next_static_chunks_main_2e8571477b54eb52_js.hpp"
#include "file_next_static_chunks_6bb80806_a5d608ad39020eee_js.hpp"
#include "file_next_static_chunks_3c1c9f4a_13941a99703194ad_js.hpp"
#include "file_next_static_chunks_webpack_9a68f0196a060eb0_js.hpp"
#include "file_next_static_chunks_main_app_e3a92c18fc983fe1_js.hpp"
#include "file_next_static_chunks_902_02a4e3e7dd6de078_js.hpp"
#include "file_next_static_chunks_199_bc7f641ac29ae1f4_js.hpp"
#include "file_next_static_chunks_8d466849_3e1c16aabe33caff_js.hpp"
#include "file_next_static_chunks_696_b636fe34b79a4e00_js.hpp"
#include "file_next_static_chunks_424_e724fd0d45236c50_js.hpp"
#include "file_next_static_chunks_29_b233dcd4cd43e265_js.hpp"
#include "file_next_static_chunks_polyfills_42372ed130431b0a_js.hpp"
#include "file_next_static_chunks_app_page_82dbdb4337f2978b_js.hpp"
#include "file_next_static_chunks_app_layout_a227b7c884d754eb_js.hpp"
#include "file_next_static_chunks_app_not_found_page_d9abfad02928c6f9_js.hpp"
#include "file_next_static_chunks_pages_error_9e91ec86137a41e3_js.hpp"
#include "file_next_static_chunks_pages_app_fd7e2f06c907c6fa_js.hpp"
#include "file_next_static_QmGGheUQ9gMdFVh6YI2KH_ssgManifest_js.hpp"
#include "file_next_static_QmGGheUQ9gMdFVh6YI2KH_buildManifest_js.hpp"
#include "file_next_static_media_7323b9d087306adb_s_p_woff2.hpp"
#include "file_next_static_media_959fe13ebd2a94e9_s_woff2.hpp"
#include "file_favicon_web_app_manifest_192x192_png.hpp"
#include "file_favicon_web_app_manifest_512x512_png.hpp"
#include "file_fonts_Urbanist_VariableFont_wght_ttf.hpp"

#include <map>

namespace duckdb { 
std::vector<File> files = {
	FILE_ICON_PNG,FILE_FAVICON_ICO,FILE_INDEX_HTML,FILE_APPLE_ICON_PNG,FILE_INDEX_TXT,FILE_404_HTML,FILE_ICON_SVG,FILE_MANIFEST_JSON,FILE_NEXT_STATIC_CSS_60B2E10D0C51DC30_CSS,FILE_NEXT_STATIC_CSS_21BB8425BD7146CC_CSS,FILE_NEXT_STATIC_CSS_5B54331129C9114C_CSS,FILE_NEXT_STATIC_CHUNKS_75146D7D_09AA1A27EF8FFD85_JS,FILE_NEXT_STATIC_CHUNKS_DC10EADF_140341D4BBE49AC0_JS,FILE_NEXT_STATIC_CHUNKS_273ACDC0_074651C6DCCE57AA_JS,FILE_NEXT_STATIC_CHUNKS_122_3EC9E1262BD14E23_JS,FILE_NEXT_STATIC_CHUNKS_93BFA88F_CC391F3208003935_JS,FILE_NEXT_STATIC_CHUNKS_872_6E627E6A293ED396_JS,FILE_NEXT_STATIC_CHUNKS_FRAMEWORK_F238C7B8F578DD52_JS,FILE_NEXT_STATIC_CHUNKS_0B42CE73_8A46A4463856ECB8_JS,FILE_NEXT_STATIC_CHUNKS_32FA3855_229C70A9F91729D1_JS,FILE_NEXT_STATIC_CHUNKS_MAIN_2E8571477B54EB52_JS,FILE_NEXT_STATIC_CHUNKS_6BB80806_A5D608AD39020EEE_JS,FILE_NEXT_STATIC_CHUNKS_3C1C9F4A_13941A99703194AD_JS,FILE_NEXT_STATIC_CHUNKS_WEBPACK_9A68F0196A060EB0_JS,FILE_NEXT_STATIC_CHUNKS_MAIN_APP_E3A92C18FC983FE1_JS,FILE_NEXT_STATIC_CHUNKS_902_02A4E3E7DD6DE078_JS,FILE_NEXT_STATIC_CHUNKS_199_BC7F641AC29AE1F4_JS,FILE_NEXT_STATIC_CHUNKS_8D466849_3E1C16AABE33CAFF_JS,FILE_NEXT_STATIC_CHUNKS_696_B636FE34B79A4E00_JS,FILE_NEXT_STATIC_CHUNKS_424_E724FD0D45236C50_JS,FILE_NEXT_STATIC_CHUNKS_29_B233DCD4CD43E265_JS,FILE_NEXT_STATIC_CHUNKS_POLYFILLS_42372ED130431B0A_JS,FILE_NEXT_STATIC_CHUNKS_APP_PAGE_82DBDB4337F2978B_JS,FILE_NEXT_STATIC_CHUNKS_APP_LAYOUT_A227B7C884D754EB_JS,FILE_NEXT_STATIC_CHUNKS_APP_NOT_FOUND_PAGE_D9ABFAD02928C6F9_JS,FILE_NEXT_STATIC_CHUNKS_PAGES_ERROR_9E91EC86137A41E3_JS,FILE_NEXT_STATIC_CHUNKS_PAGES_APP_FD7E2F06C907C6FA_JS,FILE_NEXT_STATIC_QMGGHEUQ9GMDFVH6YI2KH_SSGMANIFEST_JS,FILE_NEXT_STATIC_QMGGHEUQ9GMDFVH6YI2KH_BUILDMANIFEST_JS,FILE_NEXT_STATIC_MEDIA_7323B9D087306ADB_S_P_WOFF2,FILE_NEXT_STATIC_MEDIA_959FE13EBD2A94E9_S_WOFF2,FILE_FAVICON_WEB_APP_MANIFEST_192X192_PNG,FILE_FAVICON_WEB_APP_MANIFEST_512X512_PNG,FILE_FONTS_URBANIST_VARIABLEFONT_WGHT_TTF
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
