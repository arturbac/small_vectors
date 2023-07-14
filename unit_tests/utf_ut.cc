#include <unit_test_core.h>
#include <coll/utf/utf.h>
#include <codecvt>

#if defined(__clang__) && !defined(_LIBCPP_STD_VER)
//gnu libstdc++ violates union member access rules accessing not activated member in union in std::basic_string
// UB in consteval code is not allowed (gcc ignores that and allows UB in consteval union access)
constexpr bool enable_consteval_string_testing{false};
#else
constexpr bool enable_consteval_string_testing{true};
#endif
using metatests::constexpr_test;
using metatests::run_consteval_test;
using metatests::run_constexpr_test;
namespace ut = boost::ut;
using ut::operator""_test;
using namespace ut::operators::terse;
using metatests::test_result;
using namespace std::string_literals;

using std::ranges::begin;
using std::ranges::end;

constexpr void clear( auto & storage )
  {
  std::ranges::fill( storage, 0u );
  }
constexpr auto view( auto const & a ) noexcept { return std::basic_string_view{ std::begin(a), std::end(a)}; }
//https://en.wikipedia.org/wiki/Plane_(Unicode)

constexpr std::basic_string_view u8test{ 
                                           u8"Lorem bardzo marchewka, twórca pomidorów. Prezent i pakiet."
                                           u8"Лорем дуже морква, розробник бакалавра з томатами. ПРЕДСТАВЛЕННЯ І ПАКЕТ."
                                           u8"Lorem非常胡萝卜，番茄本科开发人员。 现在和包裹。 营养或毕业是保留的，足球以前是针对的。"
                                           u8" פאָרשטעלן און דעם פּעקל. דערנערונג אָדער גראַדזשאַווייטיד רעזערווירט, פוטבאָל טאַרגעטעד פריער."
                                           //https://en.wikibooks.org/wiki/Unicode/Character_reference/F000-FFFF
                                           u8"ﻀﻁﻂﻃﻄﻅﻆﻇﻈﻉﻊﻋﻌﻍﻎﻏﺐﺑﺒﺓﺔﺕﺖﺗﺘﺙﺚﺛﺜﺝﺞﺟ"
                                           //https://en.wikipedia.org/wiki/Linear_B_Syllabary
                                           u8"𐀀𐀁𐀂𐀃𐀄𐀅𐀆𐀇𐀈𐀉𐀊𐀋𐀍𐀎	𐀏𐀐𐀑𐀒𐀓𐀔𐀕𐀖𐀗𐀘𐀙𐀚𐀛𐀜𐀝𐀞𐀟𐀠𐀡𐀢𐀣𐀤𐀥𐀦	𐀨𐀩𐀪𐀫𐀬𐀭𐀮𐀯𐀰𐀱𐀲𐀳𐀴𐀵𐀶𐀷𐀸𐀹𐀺	𐀼𐀽𐀿𐁀𐁁𐁂𐁃𐁄𐁅𐁆𐁇𐁈𐁉𐁊𐁋𐁌𐁍𐁐𐁑𐁒𐁓𐁔𐁕𐁖𐁗𐁘𐁙𐁚𐁛𐁜𐁝"
                                           //https://en.wikipedia.org/wiki/Linear_B_Ideograms
                                           u8"𐂀𐂁𐂂𐂃𐂄𐂅𐂆𐂇𐂈𐂉𐂊𐂋𐂌𐂍𐂎𐂏𐂐𐂑𐂒𐂓𐂔𐂕𐂖𐂗𐂘𐂙𐂚𐂛𐂜𐂝𐂞𐂟𐂠𐂡𐂢𐂣𐂤𐂥𐂦𐂧𐂨𐂩𐂪𐂫𐂬𐂭𐂮𐂯𐂰𐂱𐂲𐂳𐂴𐂵𐂶𐂷𐂸𐂹𐂺𐂻𐂼𐂽𐂾𐂿𐃀𐃁𐃂𐃃𐃄𐃅"
                                           u8"𐃆𐃇𐃈𐃉𐃊𐃋𐃌𐃍𐃎𐃏𐃐𐃑𐃒𐃓𐃔𐃕𐃖𐃗𐃘𐃙𐃚𐃛𐃜𐃝𐃞𐃟𐃠𐃡𐃢𐃣𐃤𐃥𐃦𐃧𐃨𐃩𐃪𐃫𐃬𐃭𐃮𐃯𐃰𐃱𐃲𐃳𐃴𐃵𐃶𐃷𐃸𐃹𐃺"
                                           //https://en.wikipedia.org/wiki/Symbols_for_Legacy_Computing
                                           u8"🬀🬁🬂🬃🬄🬅🬆🬇🬈🬉🬊🬋🬌🬍🬎🬏🬐🬑🬒🬓🬔🬕🬖🬗🬘🬙🬚🬛🬜🬝🬞🬟🬠🬡🬢🬣🬤🬥🬦🬧🬨🬩🬪🬫🬬🬭🬮🬯🬰🬱🬲🬳🬴🬵🬶🬷🬸🬹🬺🬻🬼🬽🬾🬿"
                                           u8"🭀🭁🭂🭃🭄🭅🭆🭇🭈🭉🭊🭋🭌🭍🭎🭏🭐🭑🭒🭓🭔🭕🭖🭗🭘🭙🭚🭛🭜🭝🭞🭟🭠🭡🭢🭣🭤🭥🭦🭧🭨🭩🭪🭫🭬🭭🭮🭯🭰🭱🭲🭳🭴🭵🭶🭷🭸🭹🭺🭻🭼🭽🭾🭿"
                                           u8"🮀🮁🮂🮃🮄🮅🮆🮇🮈🮉🮊🮋🮌🮍🮎🮏🮐🮑🮒	🮔🮕🮖🮗🮘🮙🮚🮛🮜🮝🮞🮟🮠🮡🮢🮣🮤🮥🮦🮧🮨🮩🮪🮫🮬🮭🮮🮯🮰🮱🮲🮳🮴🮵🮶🮷🮸🮹🮺🮻🮼🮽🮾🮿"
                                           u8"🯀🯁🯂🯃🯄🯅🯆🯇🯈🯉🯊🯰🯱🯲🯳🯴🯵🯶🯷🯸🯹"
                                           
                              };
constexpr std::basic_string_view u16test{ 
                                           u"Lorem bardzo marchewka, twórca pomidorów. Prezent i pakiet."
                                           u"Лорем дуже морква, розробник бакалавра з томатами. ПРЕДСТАВЛЕННЯ І ПАКЕТ."
                                           u"Lorem非常胡萝卜，番茄本科开发人员。 现在和包裹。 营养或毕业是保留的，足球以前是针对的。"
                                           u" פאָרשטעלן און דעם פּעקל. דערנערונג אָדער גראַדזשאַווייטיד רעזערווירט, פוטבאָל טאַרגעטעד פריער."
                                           u"ﻀﻁﻂﻃﻄﻅﻆﻇﻈﻉﻊﻋﻌﻍﻎﻏﺐﺑﺒﺓﺔﺕﺖﺗﺘﺙﺚﺛﺜﺝﺞﺟ"
                                           u"𐀀𐀁𐀂𐀃𐀄𐀅𐀆𐀇𐀈𐀉𐀊𐀋𐀍𐀎	𐀏𐀐𐀑𐀒𐀓𐀔𐀕𐀖𐀗𐀘𐀙𐀚𐀛𐀜𐀝𐀞𐀟𐀠𐀡𐀢𐀣𐀤𐀥𐀦	𐀨𐀩𐀪𐀫𐀬𐀭𐀮𐀯𐀰𐀱𐀲𐀳𐀴𐀵𐀶𐀷𐀸𐀹𐀺	𐀼𐀽𐀿𐁀𐁁𐁂𐁃𐁄𐁅𐁆𐁇𐁈𐁉𐁊𐁋𐁌𐁍𐁐𐁑𐁒𐁓𐁔𐁕𐁖𐁗𐁘𐁙𐁚𐁛𐁜𐁝"
                                           u"𐂀𐂁𐂂𐂃𐂄𐂅𐂆𐂇𐂈𐂉𐂊𐂋𐂌𐂍𐂎𐂏𐂐𐂑𐂒𐂓𐂔𐂕𐂖𐂗𐂘𐂙𐂚𐂛𐂜𐂝𐂞𐂟𐂠𐂡𐂢𐂣𐂤𐂥𐂦𐂧𐂨𐂩𐂪𐂫𐂬𐂭𐂮𐂯𐂰𐂱𐂲𐂳𐂴𐂵𐂶𐂷𐂸𐂹𐂺𐂻𐂼𐂽𐂾𐂿𐃀𐃁𐃂𐃃𐃄𐃅"
                                           u"𐃆𐃇𐃈𐃉𐃊𐃋𐃌𐃍𐃎𐃏𐃐𐃑𐃒𐃓𐃔𐃕𐃖𐃗𐃘𐃙𐃚𐃛𐃜𐃝𐃞𐃟𐃠𐃡𐃢𐃣𐃤𐃥𐃦𐃧𐃨𐃩𐃪𐃫𐃬𐃭𐃮𐃯𐃰𐃱𐃲𐃳𐃴𐃵𐃶𐃷𐃸𐃹𐃺"
                                           u"🬀🬁🬂🬃🬄🬅🬆🬇🬈🬉🬊🬋🬌🬍🬎🬏🬐🬑🬒🬓🬔🬕🬖🬗🬘🬙🬚🬛🬜🬝🬞🬟🬠🬡🬢🬣🬤🬥🬦🬧🬨🬩🬪🬫🬬🬭🬮🬯🬰🬱🬲🬳🬴🬵🬶🬷🬸🬹🬺🬻🬼🬽🬾🬿"
                                           u"🭀🭁🭂🭃🭄🭅🭆🭇🭈🭉🭊🭋🭌🭍🭎🭏🭐🭑🭒🭓🭔🭕🭖🭗🭘🭙🭚🭛🭜🭝🭞🭟🭠🭡🭢🭣🭤🭥🭦🭧🭨🭩🭪🭫🭬🭭🭮🭯🭰🭱🭲🭳🭴🭵🭶🭷🭸🭹🭺🭻🭼🭽🭾🭿"
                                           u"🮀🮁🮂🮃🮄🮅🮆🮇🮈🮉🮊🮋🮌🮍🮎🮏🮐🮑🮒	🮔🮕🮖🮗🮘🮙🮚🮛🮜🮝🮞🮟🮠🮡🮢🮣🮤🮥🮦🮧🮨🮩🮪🮫🮬🮭🮮🮯🮰🮱🮲🮳🮴🮵🮶🮷🮸🮹🮺🮻🮼🮽🮾🮿"
                                           u"🯀🯁🯂🯃🯄🯅🯆🯇🯈🯉🯊🯰🯱🯲🯳🯴🯵🯶🯷🯸🯹"
                              };
constexpr std::basic_string_view u32test{ 
                                           U"Lorem bardzo marchewka, twórca pomidorów. Prezent i pakiet."
                                           U"Лорем дуже морква, розробник бакалавра з томатами. ПРЕДСТАВЛЕННЯ І ПАКЕТ."
                                           U"Lorem非常胡萝卜，番茄本科开发人员。 现在和包裹。 营养或毕业是保留的，足球以前是针对的。"
                                           U" פאָרשטעלן און דעם פּעקל. דערנערונג אָדער גראַדזשאַווייטיד רעזערווירט, פוטבאָל טאַרגעטעד פריער."
                                           U"ﻀﻁﻂﻃﻄﻅﻆﻇﻈﻉﻊﻋﻌﻍﻎﻏﺐﺑﺒﺓﺔﺕﺖﺗﺘﺙﺚﺛﺜﺝﺞﺟ"
                                           U"𐀀𐀁𐀂𐀃𐀄𐀅𐀆𐀇𐀈𐀉𐀊𐀋𐀍𐀎	𐀏𐀐𐀑𐀒𐀓𐀔𐀕𐀖𐀗𐀘𐀙𐀚𐀛𐀜𐀝𐀞𐀟𐀠𐀡𐀢𐀣𐀤𐀥𐀦	𐀨𐀩𐀪𐀫𐀬𐀭𐀮𐀯𐀰𐀱𐀲𐀳𐀴𐀵𐀶𐀷𐀸𐀹𐀺	𐀼𐀽𐀿𐁀𐁁𐁂𐁃𐁄𐁅𐁆𐁇𐁈𐁉𐁊𐁋𐁌𐁍𐁐𐁑𐁒𐁓𐁔𐁕𐁖𐁗𐁘𐁙𐁚𐁛𐁜𐁝"
                                           U"𐂀𐂁𐂂𐂃𐂄𐂅𐂆𐂇𐂈𐂉𐂊𐂋𐂌𐂍𐂎𐂏𐂐𐂑𐂒𐂓𐂔𐂕𐂖𐂗𐂘𐂙𐂚𐂛𐂜𐂝𐂞𐂟𐂠𐂡𐂢𐂣𐂤𐂥𐂦𐂧𐂨𐂩𐂪𐂫𐂬𐂭𐂮𐂯𐂰𐂱𐂲𐂳𐂴𐂵𐂶𐂷𐂸𐂹𐂺𐂻𐂼𐂽𐂾𐂿𐃀𐃁𐃂𐃃𐃄𐃅"
                                           U"𐃆𐃇𐃈𐃉𐃊𐃋𐃌𐃍𐃎𐃏𐃐𐃑𐃒𐃓𐃔𐃕𐃖𐃗𐃘𐃙𐃚𐃛𐃜𐃝𐃞𐃟𐃠𐃡𐃢𐃣𐃤𐃥𐃦𐃧𐃨𐃩𐃪𐃫𐃬𐃭𐃮𐃯𐃰𐃱𐃲𐃳𐃴𐃵𐃶𐃷𐃸𐃹𐃺"
                                           U"🬀🬁🬂🬃🬄🬅🬆🬇🬈🬉🬊🬋🬌🬍🬎🬏🬐🬑🬒🬓🬔🬕🬖🬗🬘🬙🬚🬛🬜🬝🬞🬟🬠🬡🬢🬣🬤🬥🬦🬧🬨🬩🬪🬫🬬🬭🬮🬯🬰🬱🬲🬳🬴🬵🬶🬷🬸🬹🬺🬻🬼🬽🬾🬿"
                                           U"🭀🭁🭂🭃🭄🭅🭆🭇🭈🭉🭊🭋🭌🭍🭎🭏🭐🭑🭒🭓🭔🭕🭖🭗🭘🭙🭚🭛🭜🭝🭞🭟🭠🭡🭢🭣🭤🭥🭦🭧🭨🭩🭪🭫🭬🭭🭮🭯🭰🭱🭲🭳🭴🭵🭶🭷🭸🭹🭺🭻🭼🭽🭾🭿"
                                           U"🮀🮁🮂🮃🮄🮅🮆🮇🮈🮉🮊🮋🮌🮍🮎🮏🮐🮑🮒	🮔🮕🮖🮗🮘🮙🮚🮛🮜🮝🮞🮟🮠🮡🮢🮣🮤🮥🮦🮧🮨🮩🮪🮫🮬🮭🮮🮯🮰🮱🮲🮳🮴🮵🮶🮷🮸🮹🮺🮻🮼🮽🮾🮿"
                                           U"🯀🯁🯂🯃🯄🯅🯆🯇🯈🯉🯊🯰🯱🯲🯳🯴🯵🯶🯷🯸🯹"
                              };
constexpr std::basic_string_view wtest{ 
                                           L"Lorem bardzo marchewka, twórca pomidorów. Prezent i pakiet."
                                           L"Лорем дуже морква, розробник бакалавра з томатами. ПРЕДСТАВЛЕННЯ І ПАКЕТ."
                                           L"Lorem非常胡萝卜，番茄本科开发人员。 现在和包裹。 营养或毕业是保留的，足球以前是针对的。"
                                           L" פאָרשטעלן און דעם פּעקל. דערנערונג אָדער גראַדזשאַווייטיד רעזערווירט, פוטבאָל טאַרגעטעד פריער."
                                           L"ﻀﻁﻂﻃﻄﻅﻆﻇﻈﻉﻊﻋﻌﻍﻎﻏﺐﺑﺒﺓﺔﺕﺖﺗﺘﺙﺚﺛﺜﺝﺞﺟ"
                                           L"𐀀𐀁𐀂𐀃𐀄𐀅𐀆𐀇𐀈𐀉𐀊𐀋𐀍𐀎	𐀏𐀐𐀑𐀒𐀓𐀔𐀕𐀖𐀗𐀘𐀙𐀚𐀛𐀜𐀝𐀞𐀟𐀠𐀡𐀢𐀣𐀤𐀥𐀦	𐀨𐀩𐀪𐀫𐀬𐀭𐀮𐀯𐀰𐀱𐀲𐀳𐀴𐀵𐀶𐀷𐀸𐀹𐀺	𐀼𐀽𐀿𐁀𐁁𐁂𐁃𐁄𐁅𐁆𐁇𐁈𐁉𐁊𐁋𐁌𐁍𐁐𐁑𐁒𐁓𐁔𐁕𐁖𐁗𐁘𐁙𐁚𐁛𐁜𐁝"
                                           L"𐂀𐂁𐂂𐂃𐂄𐂅𐂆𐂇𐂈𐂉𐂊𐂋𐂌𐂍𐂎𐂏𐂐𐂑𐂒𐂓𐂔𐂕𐂖𐂗𐂘𐂙𐂚𐂛𐂜𐂝𐂞𐂟𐂠𐂡𐂢𐂣𐂤𐂥𐂦𐂧𐂨𐂩𐂪𐂫𐂬𐂭𐂮𐂯𐂰𐂱𐂲𐂳𐂴𐂵𐂶𐂷𐂸𐂹𐂺𐂻𐂼𐂽𐂾𐂿𐃀𐃁𐃂𐃃𐃄𐃅"
                                           L"𐃆𐃇𐃈𐃉𐃊𐃋𐃌𐃍𐃎𐃏𐃐𐃑𐃒𐃓𐃔𐃕𐃖𐃗𐃘𐃙𐃚𐃛𐃜𐃝𐃞𐃟𐃠𐃡𐃢𐃣𐃤𐃥𐃦𐃧𐃨𐃩𐃪𐃫𐃬𐃭𐃮𐃯𐃰𐃱𐃲𐃳𐃴𐃵𐃶𐃷𐃸𐃹𐃺"
                                           L"🬀🬁🬂🬃🬄🬅🬆🬇🬈🬉🬊🬋🬌🬍🬎🬏🬐🬑🬒🬓🬔🬕🬖🬗🬘🬙🬚🬛🬜🬝🬞🬟🬠🬡🬢🬣🬤🬥🬦🬧🬨🬩🬪🬫🬬🬭🬮🬯🬰🬱🬲🬳🬴🬵🬶🬷🬸🬹🬺🬻🬼🬽🬾🬿"
                                           L"🭀🭁🭂🭃🭄🭅🭆🭇🭈🭉🭊🭋🭌🭍🭎🭏🭐🭑🭒🭓🭔🭕🭖🭗🭘🭙🭚🭛🭜🭝🭞🭟🭠🭡🭢🭣🭤🭥🭦🭧🭨🭩🭪🭫🭬🭭🭮🭯🭰🭱🭲🭳🭴🭵🭶🭷🭸🭹🭺🭻🭼🭽🭾🭿"
                                           L"🮀🮁🮂🮃🮄🮅🮆🮇🮈🮉🮊🮋🮌🮍🮎🮏🮐🮑🮒	🮔🮕🮖🮗🮘🮙🮚🮛🮜🮝🮞🮟🮠🮡🮢🮣🮤🮥🮦🮧🮨🮩🮪🮫🮬🮭🮮🮯🮰🮱🮲🮳🮴🮵🮶🮷🮸🮹🮺🮻🮼🮽🮾🮿"
                                           L"🯀🯁🯂🯃🯄🯅🯆🯇🯈🯉🯊🯰🯱🯲🯳🯴🯵🯶🯷🯸🯹"
                              };
[[maybe_unused]]
static char hex4( unsigned value ) 
  { 
  static const char ALPHA[] = "0123456789ABCDEF";
  return ALPHA[value];
  }
[[maybe_unused]]
static void dump_utf( auto const & result )
  {
  for( auto el : result )
    {
    std::cout  << hex4(unsigned(el)>>4) << hex4(unsigned(el) & 0xf);
    }
  std::cout << std::endl;
  }
  
namespace utf = coll::utf;
int main()
{
  using utf::u8;
  
  test_result result;
  "sequence_length"_test = [&]
    {
    auto fn_tmpl = [] () -> metatests::test_result
      {
      constexpr_test( 1 == utf::detail::sequence_length( char8_t(0b0101'0000) ));
      constexpr_test( 2 == utf::detail::sequence_length( char8_t(0b1101'1000) ));
      constexpr_test( 3 == utf::detail::sequence_length( char8_t(0b1110'1000) ));
      constexpr_test( 4 == utf::detail::sequence_length( char8_t(0b1111'0110) ));
      return {};
      };
    result |= run_constexpr_test(fn_tmpl);
    result |= run_consteval_test(fn_tmpl);
    };

  "verify"_test = [&]
    {
    auto fn_tmpl = [] () -> metatests::test_result
      {
      using enum utf::detail::verify_status_e;
      constexpr_test( utf::verify(u8test) == valid );
      // test overlength
      {
        // ARABIC LETTER JEEM ISOLATED FORM 
        // FE9D ﺝ
        // 0xEFBA9D
        // 0b1111'111010'011101
        // proper encoding
        // 0b1110'1111 0b10'111010 0b10'011101
        // overlength
        // 0b11110'000  10'001111 10'111010 10'011101
        std::array<char8_t,4> ic{ 0b11110000, 0b10'001111, 0b10'111010, 0b10'011101 };
        constexpr_test( utf::verify(ic) == overlength_code_point);
        constexpr_test( utf::detail::dereference(ic.begin()) == char32_t(0xFE9D));
      }
      {
        // invalid code point 0x17FFFF
        // 0b11110'101 0b10'111111 0b10'111111 0b10'111111
        std::array<char8_t,4> ic{ 0b11110'101, 0b10'111111, 0b10'111111, 0b10'111111 };
        constexpr_test( utf::verify(ic) == invalid_code_point);
      }
      {
        // missing trail
        //   0b1110'1111 0b10'111010 0b10'011101 
        //-> 0b1110'1111 0b10'111010 0b00'011101
        std::array<char8_t,3> ic{ 0b1110'1111, 0b10'111010, 0b00'011101 };
        constexpr_test( utf::verify(ic) == invalid_trail);
      }
      {
        // truncated
        //   0b1110'1111 0b10'111010 0b10'011101 
        //-> 0b1110'1111 0b10'111010
        std::array<char8_t,2> ic{ 0b1110'1111, 0b10'111010 };
        constexpr_test( utf::verify(ic) == truncated_code_point);
      }
      {
        // invalid_lead 0b111110
        std::array<char8_t,4> ic{ 0b111110'00, 0b10'001111, 0b10'111010, 0b10'011101 };
        constexpr_test( utf::verify(ic) == invalid_lead);
      }
      return {};
      };

    result |= run_constexpr_test(fn_tmpl);
    result |= run_consteval_test(fn_tmpl);
    };

  "length"_test = [&]
    {
    auto fn_tmpl = [] () -> metatests::test_result
      {
      constexpr_test( utf::length(u8test) == u32test.size() );
      constexpr_test( utf::length(u16test) == u32test.size() );
      constexpr_test( utf::length(u32test) == u32test.size() );
      constexpr_test( utf::length(wtest) == u32test.size() );
      return {};
      };

    result |= run_constexpr_test(fn_tmpl);
    result |= run_consteval_test(fn_tmpl);
    };
    
  "capacity"_test = [&]
    {
    auto fn_tmpl = [] () -> metatests::test_result
      {
      constexpr_test( utf::u8capacity(u8test) == u8test.size() );
      constexpr_test( utf::u8capacity(u16test) == u8test.size() );
      constexpr_test( utf::u8capacity(u32test) == u8test.size() );
      constexpr_test( utf::u8capacity(wtest) == u8test.size() );
      
      constexpr_test( utf::u16capacity(u8test) == u16test.size() );
      constexpr_test( utf::u16capacity(u16test) == u16test.size() );
      constexpr_test( utf::u16capacity(u32test) == u16test.size() );
      constexpr_test( utf::u16capacity(wtest) == u16test.size() );
      
      constexpr_test( utf::u32capacity(u8test) == u32test.size() );
      constexpr_test( utf::u32capacity(u16test) == u32test.size() );
      constexpr_test( utf::u32capacity(u32test) == u32test.size() );
      constexpr_test( utf::u32capacity(wtest) == u32test.size() );
      return {};
      };

    result |= run_constexpr_test(fn_tmpl);
    result |= run_consteval_test(fn_tmpl);
    };

  "to_char16_t"_test = [&]
    {
    auto fn_tmpl = [] () -> metatests::test_result
      {
      std::array<char16_t, u16test.size()> storage;

      // to char16_t
      utf::convert( u8test, begin(storage));
      constexpr_test( view(storage) == u16test );
      
      clear(storage);
      utf::convert( u32test, begin(storage));
      constexpr_test( view(storage) == u16test );
      
      clear(storage);
      utf::convert( wtest, begin(storage));
      constexpr_test( view(storage) == u16test );
      return {};
      };
    
      result |= run_constexpr_test(fn_tmpl);
      result |= run_consteval_test(fn_tmpl);
    };
    
  "to_u16string"_test = [&]
    {
    auto fn_tmpl = [] () -> metatests::test_result
      {
      constexpr_test( utf::to_u16string(u8test) == u16test );
      constexpr_test( utf::to_u16string(u32test) == u16test );
      constexpr_test( utf::to_u16string(wtest) == u16test );
      
      if (!std::is_constant_evaluated() || enable_consteval_string_testing )
        {
        constexpr_test( utf::stl::to_u16string(u8test) == u16test );
        constexpr_test( utf::stl::to_u16string(u32test) == u16test );
        constexpr_test( utf::stl::to_u16string(wtest) == u16test );
        }
      return {};
      };
      result |= run_constexpr_test(fn_tmpl);
      result |= run_consteval_test(fn_tmpl);
    };
    
  "to_char32_t"_test = [&]
    {
    auto fn_tmpl = [] () -> metatests::test_result
      {
      std::array<char32_t, u32test.size()> storage;
      // to char32_t
      utf::convert( u8test, begin(storage));
      constexpr_test( view(storage) == u32test );
      
      clear(storage);
      utf::convert( u16test, begin(storage));
      constexpr_test( view(storage) == u32test );
      
      return {};
      };
    
      result |= run_constexpr_test(fn_tmpl);
      result |= run_consteval_test(fn_tmpl);
    };
    
  "to_u32string"_test = [&]
    {
    auto fn_tmpl = [] () -> metatests::test_result
      {
      constexpr_test( utf::to_u32string(u8test) == u32test );
      constexpr_test( utf::to_u32string(u16test) == u32test );
      
      if (!std::is_constant_evaluated() || enable_consteval_string_testing )
        {
        constexpr_test( utf::stl::to_u32string(u8test) == u32test );
        constexpr_test( utf::stl::to_u32string(u16test) == u32test );
        }
      return {};
      };
      result |= run_constexpr_test(fn_tmpl);
      result |= run_consteval_test(fn_tmpl);
    };
    
  "to_wchar_t"_test = [&]
    {
    auto fn_tmpl = [] () -> metatests::test_result
      {
      std::array<wchar_t, wtest.size()> storage;
      // to wchar_t
      utf::convert( u8test, begin(storage));
      constexpr_test( view(storage) == wtest );
      
      clear(storage);
      utf::convert( u16test, begin(storage));
      constexpr_test( view(storage) == wtest );
      return {};
      };
    
      result |= run_constexpr_test(fn_tmpl);
      result |= run_consteval_test(fn_tmpl);
    };

  "to_wstring"_test = [&]
    {
    auto fn_tmpl = [] () -> metatests::test_result
      {
      constexpr_test( utf::to_wstring(u8test) == wtest );
      constexpr_test( utf::to_wstring(u16test) == wtest );
      
      if (!std::is_constant_evaluated() || enable_consteval_string_testing )
        {
        constexpr_test( utf::stl::to_wstring(u8test) == wtest );
        constexpr_test( utf::stl::to_wstring(u16test) == wtest );
        }
      return {};
      };
      result |= run_constexpr_test(fn_tmpl);
      result |= run_consteval_test(fn_tmpl);
    };
    
  "to_char8_t"_test = [&]
    {
    auto fn_tmpl = [] () -> metatests::test_result
      {
      std::array<char8_t, u8test.size()> storage{};

      utf::convert( u32test, begin(storage));
      constexpr_test( view(storage) == u8test );
      
      clear(storage);
      utf::convert( u16test, begin(storage));
      constexpr_test( view(storage) == u8test );

      clear(storage);
      utf::convert( wtest, begin(storage));
      constexpr_test( view(storage) == u8test );
      return {};
      };
      result |= run_constexpr_test(fn_tmpl);
      result |= run_consteval_test(fn_tmpl);
    };
    
  "to_u8string"_test = [&]
    {
    auto fn_tmpl = [] () -> metatests::test_result
      {
      constexpr_test( utf::to_u8string(u16test) == u8test);
      constexpr_test( utf::to_u8string(u32test) == u8test);
      constexpr_test( utf::to_u8string(wtest) == u8test);
      if (!std::is_constant_evaluated() || enable_consteval_string_testing )
        {
        constexpr_test( utf::stl::to_u8string(u16test) == u8test);
        constexpr_test( utf::stl::to_u8string(u32test) == u8test);
        constexpr_test( utf::stl::to_u8string(wtest) == u8test);
        }
      return {};
      };
      result |= run_constexpr_test(fn_tmpl);
      result |= run_consteval_test(fn_tmpl);
    };
}

// convert( begin(u8t), end(u8t), begin(storage));
// std::cout << std::endl;
// dump_utf(view(storage));
// dump_utf(u8test);

// utf_input_view_t v2{begin(u8t), end(u8t)};
// for( char32_t cp : v2 )
//   {
//   std::array<char8_t,4> tmp{};
//   auto end_it{coll::utf::detail::append(cp, tmp.begin())};
//   for(auto it(begin(tmp)); it != end_it; ++it)
//     std::cout << hex4(unsigned(*it) & 0xf) << hex4(unsigned(*it)>>4);
//   std::cout << "|";
//   }
// std::cout << std::endl;
