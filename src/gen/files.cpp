#include "files.hpp"

#include "duckdb/common/optional_ptr.hpp"
#include "duckdb/common/string_util.hpp"

#include "_icon_png_.hpp"
#include "_favicon_ico_.hpp"
#include "_index_html_.hpp"
#include "_apple_icon_png_.hpp"
#include "_index_txt_.hpp"
#include "_404_html_.hpp"
#include "_icon_svg_.hpp"
#include "_manifest_json_.hpp"
#include "__next_static_V3WHJ4VoVzFaS1OtQ3sYX__ssgManifest_js_.hpp"
#include "__next_static_V3WHJ4VoVzFaS1OtQ3sYX__buildManifest_js_.hpp"
#include "__next_static_css_7f049ec031a58df3_css_.hpp"
#include "__next_static_css_06037a4dc34b6415_css_.hpp"
#include "__next_static_css_19c8296a0f775ccd_css_.hpp"
#include "__next_static_chunks_6bb80806_1baed5c03ff54307_js_.hpp"
#include "__next_static_chunks_main_app_22d963ef3fe53e14_js_.hpp"
#include "__next_static_chunks_0b42ce73_468184a4882d6ce8_js_.hpp"
#include "__next_static_chunks_main_ee9d5435d58fee8d_js_.hpp"
#include "__next_static_chunks_313_b5abe9683aa28e72_js_.hpp"
#include "__next_static_chunks_93bfa88f_6ee39640c9fe86a0_js_.hpp"
#include "__next_static_chunks_736_9bce7fde2afcc044_js_.hpp"
#include "__next_static_chunks_726_1670c22d091097cd_js_.hpp"
#include "__next_static_chunks_framework_ded83d71b51ce901_js_.hpp"
#include "__next_static_chunks_301_ebfecc23431826f9_js_.hpp"
#include "__next_static_chunks_493ab29e_5a13789b21fa6218_js_.hpp"
#include "__next_static_chunks_764_903eb5e7f2b2f294_js_.hpp"
#include "__next_static_chunks_c78ac0aa_3ace17cad8c90898_js_.hpp"
#include "__next_static_chunks_webpack_f55689b1fccb6653_js_.hpp"
#include "__next_static_chunks_polyfills_78c92fac7aa8fdd8_js_.hpp"
#include "__next_static_chunks_218_60f6896a1e91a266_js_.hpp"
#include "__next_static_chunks_584_56872ac185de22fc_js_.hpp"
#include "__next_static_chunks_240_cde431422ad46ab8_js_.hpp"
#include "__next_static_chunks_app_page_dc074af08576c246_js_.hpp"
#include "__next_static_chunks_app_loading_8f435599db3e2af4_js_.hpp"
#include "__next_static_chunks_app_layout_9c80af1e7371edd6_js_.hpp"
#include "__next_static_chunks_app__not_found_page_5fae1d7300910d3e_js_.hpp"
#include "__next_static_chunks_pages__error_f251a2e1c1b3c71c_js_.hpp"
#include "__next_static_chunks_pages__app_5db5e40c01741300_js_.hpp"
#include "__next_static_media_d1773e83ecd9f5ec_s_woff2_.hpp"
#include "__next_static_media_0b42914f8036f313_s_p_woff2_.hpp"
#include "_favicon_web_app_manifest_192x192_png_.hpp"
#include "_favicon_web_app_manifest_512x512_png_.hpp"
#include "_fonts_Urbanist_VariableFont_wght_ttf_.hpp"

#include <map>

namespace duckdb { 
std::vector<File> files = {
	_ICON_PNG_,_FAVICON_ICO_,_INDEX_HTML_,_APPLE_ICON_PNG_,_INDEX_TXT_,_404_HTML_,_ICON_SVG_,_MANIFEST_JSON_,__NEXT_STATIC_V3WHJ4VOVZFAS1OTQ3SYX__SSGMANIFEST_JS_,__NEXT_STATIC_V3WHJ4VOVZFAS1OTQ3SYX__BUILDMANIFEST_JS_,__NEXT_STATIC_CSS_7F049EC031A58DF3_CSS_,__NEXT_STATIC_CSS_06037A4DC34B6415_CSS_,__NEXT_STATIC_CSS_19C8296A0F775CCD_CSS_,__NEXT_STATIC_CHUNKS_6BB80806_1BAED5C03FF54307_JS_,__NEXT_STATIC_CHUNKS_MAIN_APP_22D963EF3FE53E14_JS_,__NEXT_STATIC_CHUNKS_0B42CE73_468184A4882D6CE8_JS_,__NEXT_STATIC_CHUNKS_MAIN_EE9D5435D58FEE8D_JS_,__NEXT_STATIC_CHUNKS_313_B5ABE9683AA28E72_JS_,__NEXT_STATIC_CHUNKS_93BFA88F_6EE39640C9FE86A0_JS_,__NEXT_STATIC_CHUNKS_736_9BCE7FDE2AFCC044_JS_,__NEXT_STATIC_CHUNKS_726_1670C22D091097CD_JS_,__NEXT_STATIC_CHUNKS_FRAMEWORK_DED83D71B51CE901_JS_,__NEXT_STATIC_CHUNKS_301_EBFECC23431826F9_JS_,__NEXT_STATIC_CHUNKS_493AB29E_5A13789B21FA6218_JS_,__NEXT_STATIC_CHUNKS_764_903EB5E7F2B2F294_JS_,__NEXT_STATIC_CHUNKS_C78AC0AA_3ACE17CAD8C90898_JS_,__NEXT_STATIC_CHUNKS_WEBPACK_F55689B1FCCB6653_JS_,__NEXT_STATIC_CHUNKS_POLYFILLS_78C92FAC7AA8FDD8_JS_,__NEXT_STATIC_CHUNKS_218_60F6896A1E91A266_JS_,__NEXT_STATIC_CHUNKS_584_56872AC185DE22FC_JS_,__NEXT_STATIC_CHUNKS_240_CDE431422AD46AB8_JS_,__NEXT_STATIC_CHUNKS_APP_PAGE_DC074AF08576C246_JS_,__NEXT_STATIC_CHUNKS_APP_LOADING_8F435599DB3E2AF4_JS_,__NEXT_STATIC_CHUNKS_APP_LAYOUT_9C80AF1E7371EDD6_JS_,__NEXT_STATIC_CHUNKS_APP__NOT_FOUND_PAGE_5FAE1D7300910D3E_JS_,__NEXT_STATIC_CHUNKS_PAGES__ERROR_F251A2E1C1B3C71C_JS_,__NEXT_STATIC_CHUNKS_PAGES__APP_5DB5E40C01741300_JS_,__NEXT_STATIC_MEDIA_D1773E83ECD9F5EC_S_WOFF2_,__NEXT_STATIC_MEDIA_0B42914F8036F313_S_P_WOFF2_,_FAVICON_WEB_APP_MANIFEST_192X192_PNG_,_FAVICON_WEB_APP_MANIFEST_512X512_PNG_,_FONTS_URBANIST_VARIABLEFONT_WGHT_TTF_
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
