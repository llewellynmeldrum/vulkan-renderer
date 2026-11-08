
#include <print>

struct Bar{
    int field{1};
};
struct Foo{
    Foo(nullptr_t){
        std::println("nullptr_t wins");
    }
    Foo(Bar* bar_ptr){
        std::println("bar_ptr wins");
    }
};
int main(){
    auto b = Bar{20};
    auto c = static_cast<Bar*>(nullptr);
    std::print("With nullptr ->");
    auto with_nullptr = Foo(nullptr);
    std::print("With bar ptr ->");
    auto with_bar_ptr = Foo(&b);

    std::print("With null_bar ptr ->");
    auto with_null_bar_ptr = Foo(c);
}

//#include <print>
//#include <unordered_map>
//
//enum KeyCode{
//    UNKNOWN = 0,
//
//    /**
//     *  \name Usage page 0x07
//     *
//     *  These values are from usage page 0x07 (USB keyboard page).
//     */
//    /* @{ */
//
//    A = 4,
//    B = 5,
//    C = 6,
//    D = 7,
//    E = 8,
//    F = 9,
//    G = 10,
//    H = 11,
//    I = 12,
//    J = 13,
//    K = 14,
//    L = 15,
//    M = 16,
//    N = 17,
//    O = 18,
//    P = 19,
//    Q = 20,
//    R = 21,
//    S = 22,
//    T = 23,
//    U = 24,
//    V = 25,
//    W = 26,
//    X = 27,
//    Y = 28,
//    Z = 29,
//
//    ONE = 30,
//    TWO = 31,
//    THREE = 32,
//    FOUR= 33,
//    FIVE = 34,
//    SIX = 35,
//    SEVEN= 36,
//    EIGHT= 37,
//    NINE = 38,
//    ZERO = 39,
//
//    RETURN = 40,
//    ESCAPE = 41,
//    BACKSPACE = 42,
//    TAB = 43,
//    SPACE = 44,
//
//    MINUS = 45,
//    EQUALS = 46,
//    LEFTBRACKET = 47,
//    RIGHTBRACKET = 48,
//    BACKSLASH = 49, /**< Located at the lower left of the return
//                                  *   key on ISO keyboards and at the right end
//                                  *   of the QWERTY row on ANSI keyboards.
//                                  *   Produces REVERSE SOLIDUS (backslash) and
//                                  *   VERTICAL LINE in a US layout, REVERSE
//                                  *   SOLIDUS and VERTICAL LINE in a UK Mac
//                                  *   layout, NUMBER SIGN and TILDE in a UK
//                                  *   Windows layout, DOLLAR SIGN and POUND SIGN
//                                  *   in a Swiss German layout, NUMBER SIGN and
//                                  *   APOSTROPHE in a German layout, GRAVE
//                                  *   ACCENT and POUND SIGN in a French Mac
//                                  *   layout, and ASTERISK and MICRO SIGN in a
//                                  *   French Windows layout.
//                                  */
//    NONUSHASH = 50, /**< ISO USB keyboards actually use this code
//                                  *   instead of 49 for the same key, but all
//                                  *   OSes I've seen treat the two codes
//                                  *   identically. So, as an implementer, unless
//                                  *   your keyboard generates both of those
//                                  *   codes and your OS treats them differently,
//                                  *   you should generate BACKSLASH
//                                  *   instead of this code. As a user, you
//                                  *   should not rely on this code because SDL
//                                  *   will never generate it with most (all?)
//                                  *   keyboards.
//                                  */
//    SEMICOLON = 51,
//    APOSTROPHE = 52,
//    GRAVE = 53, /**< Located in the top left corner (on both ANSI
//                              *   and ISO keyboards). Produces GRAVE ACCENT and
//                              *   TILDE in a US Windows layout and in US and UK
//                              *   Mac layouts on ANSI keyboards, GRAVE ACCENT
//                              *   and NOT SIGN in a UK Windows layout, SECTION
//                              *   SIGN and PLUS-MINUS SIGN in US and UK Mac
//                              *   layouts on ISO keyboards, SECTION SIGN and
//                              *   DEGREE SIGN in a Swiss German layout (Mac:
//                              *   only on ISO keyboards), CIRCUMFLEX ACCENT and
//                              *   DEGREE SIGN in a German layout (Mac: only on
//                              *   ISO keyboards), SUPERSCRIPT TWO and TILDE in a
//                              *   French Windows layout, COMMERCIAL AT and
//                              *   NUMBER SIGN in a French Mac layout on ISO
//                              *   keyboards, and LESS-THAN SIGN and GREATER-THAN
//                              *   SIGN in a Swiss German, German, or French Mac
//                              *   layout on ANSI keyboards.
//                              */
//    COMMA = 54,
//    PERIOD = 55,
//    SLASH = 56,
//
//    CAPSLOCK = 57,
//
//    F1 = 58,
//    F2 = 59,
//    F3 = 60,
//    F4 = 61,
//    F5 = 62,
//    F6 = 63,
//    F7 = 64,
//    F8 = 65,
//    F9 = 66,
//    F10 = 67,
//    F11 = 68,
//    F12 = 69,
//
//    PRINTSCREEN = 70,
//    SCROLLLOCK = 71,
//    PAUSE = 72,
//    INSERT = 73, /**< insert on PC, help on some Mac keyboards (but
//                                   does send code 73, not 117) */
//    HOME = 74,
//    PAGEUP = 75,
//    DELETE = 76,
//    END = 77,
//    PAGEDOWN = 78,
//    RIGHT = 79,
//    LEFT = 80,
//    DOWN = 81,
//    UP = 82,
//
//    NUMLOCKCLEAR = 83, /**< num lock on PC, clear on Mac keyboards
//                                     */
//    KP_DIVIDE = 84,
//    KP_MULTIPLY = 85,
//    KP_MINUS = 86,
//    KP_PLUS = 87,
//    KP_ENTER = 88,
//    KP_1 = 89,
//    KP_2 = 90,
//    KP_3 = 91,
//    KP_4 = 92,
//    KP_5 = 93,
//    KP_6 = 94,
//    KP_7 = 95,
//    KP_8 = 96,
//    KP_9 = 97,
//    KP_0 = 98,
//    KP_PERIOD = 99,
//
//    NONUSBACKSLASH = 100, /**< This is the additional key that ISO
//                                        *   keyboards have over ANSI ones,
//                                        *   located between left shift and Z.
//                                        *   Produces GRAVE ACCENT and TILDE in a
//                                        *   US or UK Mac layout, REVERSE SOLIDUS
//                                        *   (backslash) and VERTICAL LINE in a
//                                        *   US or UK Windows layout, and
//                                        *   LESS-THAN SIGN and GREATER-THAN SIGN
//                                        *   in a Swiss German, German, or French
//                                        *   layout. */
//    APPLICATION = 101, /**< windows contextual menu, compose */
//    POWER = 102, /**< The USB document says this is a status flag,
//                               *   not a physical key - but some Mac keyboards
//                               *   do have a power key. */
//    KP_EQUALS = 103,
//    F13 = 104,
//    F14 = 105,
//    F15 = 106,
//    F16 = 107,
//    F17 = 108,
//    F18 = 109,
//    F19 = 110,
//    F20 = 111,
//    F21 = 112,
//    F22 = 113,
//    F23 = 114,
//    F24 = 115,
//    EXECUTE = 116,
//    HELP = 117,    /**< AL Integrated Help Center */
//    MENU = 118,    /**< Menu (show menu) */
//    SELECT = 119,
//    STOP = 120,    /**< AC Stop */
//    AGAIN = 121,   /**< AC Redo/Repeat */
//    UNDO = 122,    /**< AC Undo */
//    CUT = 123,     /**< AC Cut */
//    COPY = 124,    /**< AC Copy */
//    PASTE = 125,   /**< AC Paste */
//    FIND = 126,    /**< AC Find */
//    MUTE = 127,
//    VOLUMEUP = 128,
//    VOLUMEDOWN = 129,
///* not sure whether there's a reason to enable these */
///*     LOCKINGCAPSLOCK = 130,  */
///*     LOCKINGNUMLOCK = 131, */
///*     LOCKINGSCROLLLOCK = 132, */
//    KP_COMMA = 133,
//    KP_EQUALSAS400 = 134,
//
//    INTERNATIONAL1 = 135, /**< used on Asian keyboards, see
//                                            footnotes in USB doc */
//    INTERNATIONAL2 = 136,
//    INTERNATIONAL3 = 137, /**< Yen */
//    INTERNATIONAL4 = 138,
//    INTERNATIONAL5 = 139,
//    INTERNATIONAL6 = 140,
//    INTERNATIONAL7 = 141,
//    INTERNATIONAL8 = 142,
//    INTERNATIONAL9 = 143,
//    LANG1 = 144, /**< Hangul/English toggle */
//    LANG2 = 145, /**< Hanja conversion */
//    LANG3 = 146, /**< Katakana */
//    LANG4 = 147, /**< Hiragana */
//    LANG5 = 148, /**< Zenkaku/Hankaku */
//    LANG6 = 149, /**< reserved */
//    LANG7 = 150, /**< reserved */
//    LANG8 = 151, /**< reserved */
//    LANG9 = 152, /**< reserved */
//
//    ALTERASE = 153,    /**< Erase-Eaze */
//    SYSREQ = 154,
//    CANCEL = 155,      /**< AC Cancel */
//    CLEAR = 156,
//    PRIOR = 157,
//    RETURN2 = 158,
//    SEPARATOR = 159,
//    OUT = 160,
//    OPER = 161,
//    CLEARAGAIN = 162,
//    CRSEL = 163,
//    EXSEL = 164,
//
//    KP_00 = 176,
//    KP_000 = 177,
//    THOUSANDSSEPARATOR = 178,
//    DECIMALSEPARATOR = 179,
//    CURRENCYUNIT = 180,
//    CURRENCYSUBUNIT = 181,
//    KP_LEFTPAREN = 182,
//    KP_RIGHTPAREN = 183,
//    KP_LEFTBRACE = 184,
//    KP_RIGHTBRACE = 185,
//    KP_TAB = 186,
//    KP_BACKSPACE = 187,
//    KP_A = 188,
//    KP_B = 189,
//    KP_C = 190,
//    KP_D = 191,
//    KP_E = 192,
//    KP_F = 193,
//    KP_XOR = 194,
//    KP_POWER = 195,
//    KP_PERCENT = 196,
//    KP_LESS = 197,
//    KP_GREATER = 198,
//    KP_AMPERSAND = 199,
//    KP_DBLAMPERSAND = 200,
//    KP_VERTICALBAR = 201,
//    KP_DBLVERTICALBAR = 202,
//    KP_COLON = 203,
//    KP_HASH = 204,
//    KP_SPACE = 205,
//    KP_AT = 206,
//    KP_EXCLAM = 207,
//    KP_MEMSTORE = 208,
//    KP_MEMRECALL = 209,
//    KP_MEMCLEAR = 210,
//    KP_MEMADD = 211,
//    KP_MEMSUBTRACT = 212,
//    KP_MEMMULTIPLY = 213,
//    KP_MEMDIVIDE = 214,
//    KP_PLUSMINUS = 215,
//    KP_CLEAR = 216,
//    KP_CLEARENTRY = 217,
//    KP_BINARY = 218,
//    KP_OCTAL = 219,
//    KP_DECIMAL = 220,
//    KP_HEXADECIMAL = 221,
//
//    LCTRL = 224,
//    LSHIFT = 225,
//    LALT = 226, /**< alt, option */
//    LGUI = 227, /**< windows, command (apple), meta */
//    RCTRL = 228,
//    RSHIFT = 229,
//    RALT = 230, /**< alt gr, option */
//    RGUI = 231, /**< windows, command (apple), meta */
//
//    MODE = 257,    /**< I'm not sure if this is really not covered
//                                 *   by any of the above, but since there's a
//                                 *   special SDL_KMOD_MODE for it I'm adding it here
//                                 */
//
//    /* @} *//* Usage page 0x07 */
//
//    /**
//     *  \name Usage page 0x0C
//     *
//     *  These values are mapped from usage page 0x0C (USB consumer page).
//     *
//     *  There are way more keys in the spec than we can represent in the
//     *  current scancode range, so pick the ones that commonly come up in
//     *  real world usage.
//     */
//    /* @{ */
//
//    SLEEP = 258,                   /**< Sleep */
//    WAKE = 259,                    /**< Wake */
//
//    CHANNEL_INCREMENT = 260,       /**< Channel Increment */
//    CHANNEL_DECREMENT = 261,       /**< Channel Decrement */
//
//    MEDIA_PLAY = 262,          /**< Play */
//    MEDIA_PAUSE = 263,         /**< Pause */
//    MEDIA_RECORD = 264,        /**< Record */
//    MEDIA_FAST_FORWARD = 265,  /**< Fast Forward */
//    MEDIA_REWIND = 266,        /**< Rewind */
//    MEDIA_NEXT_TRACK = 267,    /**< Next Track */
//    MEDIA_PREVIOUS_TRACK = 268, /**< Previous Track */
//    MEDIA_STOP = 269,          /**< Stop */
//    MEDIA_EJECT = 270,         /**< Eject */
//    MEDIA_PLAY_PAUSE = 271,    /**< Play / Pause */
//    MEDIA_SELECT = 272,        /* Media Select */
//
//    AC_NEW = 273,              /**< AC New */
//    AC_OPEN = 274,             /**< AC Open */
//    AC_CLOSE = 275,            /**< AC Close */
//    AC_EXIT = 276,             /**< AC Exit */
//    AC_SAVE = 277,             /**< AC Save */
//    AC_PRINT = 278,            /**< AC Print */
//    AC_PROPERTIES = 279,       /**< AC Properties */
//
//    AC_SEARCH = 280,           /**< AC Search */
//    AC_HOME = 281,             /**< AC Home */
//    AC_BACK = 282,             /**< AC Back */
//    AC_FORWARD = 283,          /**< AC Forward */
//    AC_STOP = 284,             /**< AC Stop */
//    AC_REFRESH = 285,          /**< AC Refresh */
//    AC_BOOKMARKS = 286,        /**< AC Bookmarks */
//
//    /* @} *//* Usage page 0x0C */
//
//
//    /**
//     *  \name Mobile keys
//     *
//     *  These are values that are often used on mobile phones.
//     */
//    /* @{ */
//
//    SOFTLEFT = 287, /**< Usually situated below the display on phones and
//                                      used as a multi-function feature key for selecting
//                                      a software defined function shown on the bottom left
//                                      of the display. */
//    SOFTRIGHT = 288, /**< Usually situated below the display on phones and
//                                       used as a multi-function feature key for selecting
//                                       a software defined function shown on the bottom right
//                                       of the display. */
//    CALL = 289, /**< Used for accepting phone calls. */
//    ENDCALL = 290, /**< Used for rejecting phone calls. */
//
//    /* @} *//* Mobile keys */
//
//    /* Add any other keys here. */
//
//    RESERVED = 400,    /**< 400-500 reserved for dynamic keycodes */
//
//    COUNT = 512 /**< not a key, just marks the number of scancodes for array bounds */
//
//};
//std::unordered_map<int,const char*> map = std::unordered_map<int,const char*>{
//
//{0,"UNKNOWN"},
//
//    /**
//     *  \name Usage page 0x07
//     *
//     *  These values are from usage page 0x07 (USB keyboard page).
//     */
//    /* @{ */
//
//{4,"A"},
//{5,"B"},
//{6,"C"},
//{7,"D"},
//{8,"E"},
//{9,"F"},
//{10,"G"},
//{11,"H"},
//{12,"I"},
//{13,"J"},
//{14,"K"},
//{15,"L"},
//{16,"M"},
//{17,"N"},
//{18,"O"},
//{19,"P"},
//{20,"Q"},
//{21,"R"},
//{22,"S"},
//{23,"T"},
//{24,"U"},
//{25,"V"},
//{26,"W"},
//{27,"X"},
//{28,"Y"},
//{29,"Z"},
//
//{30,"ONE"},
//{31,"TWO"},
//{32,"THREE"},
//    {33,"FOUR"},
//{34,"FIVE"},
//{35,"SIX"},
//    {36,"SEVEN"},
//    {37,"EIGHT"},
//{38,"NINE"},
//{39,"ZERO"},
//
//{40,"RETURN"},
//{41,"ESCAPE"},
//{42,"BACKSPACE"},
//{43,"TAB"},
//{44,"SPACE"},
//
//{45,"MINUS"},
//{46,"EQUALS"},
//{47,"LEFTBRACKET"},
//{48,"RIGHTBRACKET"},
//{49,"BACKSLASH"}, /**< Located at the lower left of the return
//                                  *   key on ISO keyboards and at the right end
//                                  *   of the QWERTY row on ANSI keyboards.
//                                  *   Produces REVERSE SOLIDUS (backslash) and
//                                  *   VERTICAL LINE in a US layout, REVERSE
//                                  *   SOLIDUS and VERTICAL LINE in a UK Mac
//                                  *   layout, NUMBER SIGN and TILDE in a UK
//                                  *   Windows layout, DOLLAR SIGN and POUND SIGN
//                                  *   in a Swiss German layout, NUMBER SIGN and
//                                  *   APOSTROPHE in a German layout, GRAVE
//                                  *   ACCENT and POUND SIGN in a French Mac
//                                  *   layout, and ASTERISK and MICRO SIGN in a
//                                  *   French Windows layout.
//                                  */
//{50,"NONUSHASH"}, /**< ISO USB keyboards actually use this code
//                                  *   instead of 49 for the same key, but all
//                                  *   OSes I've seen treat the two codes
//                                  *   identically. So, as an implementer, unless
//                                  *   your keyboard generates both of those
//                                  *   codes and your OS treats them differently,
//                                  *   you should generate BACKSLASH
//                                  *   instead of this code. As a user, you
//                                  *   should not rely on this code because SDL
//                                  *   will never generate it with most (all?)
//                                  *   keyboards.
//                                  */
//{51,"SEMICOLON"},
//{52,"APOSTROPHE"},
//{53,"GRAVE"}, /**< Located in the top left corner (on both ANSI
//                              *   and ISO keyboards). Produces GRAVE ACCENT and
//                              *   TILDE in a US Windows layout and in US and UK
//                              *   Mac layouts on ANSI keyboards, GRAVE ACCENT
//                              *   and NOT SIGN in a UK Windows layout, SECTION
//                              *   SIGN and PLUS-MINUS SIGN in US and UK Mac
//                              *   layouts on ISO keyboards, SECTION SIGN and
//                              *   DEGREE SIGN in a Swiss German layout (Mac:
//                              *   only on ISO keyboards), CIRCUMFLEX ACCENT and
//                              *   DEGREE SIGN in a German layout (Mac: only on
//                              *   ISO keyboards), SUPERSCRIPT TWO and TILDE in a
//                              *   French Windows layout, COMMERCIAL AT and
//                              *   NUMBER SIGN in a French Mac layout on ISO
//                              *   keyboards, and LESS-THAN SIGN and GREATER-THAN
//                              *   SIGN in a Swiss German, German, or French Mac
//                              *   layout on ANSI keyboards.
//                              */
//{54,"COMMA"},
//{55,"PERIOD"},
//{56,"SLASH"},
//
//{57,"CAPSLOCK"},
//
//{58,"F1"},
//{59,"F2"},
//{60,"F3"},
//{61,"F4"},
//{62,"F5"},
//{63,"F6"},
//{64,"F7"},
//{65,"F8"},
//{66,"F9"},
//{67,"F10"},
//{68,"F11"},
//{69,"F12"},
//
//{70,"PRINTSCREEN"},
//{71,"SCROLLLOCK"},
//{72,"PAUSE"},
//{73,"INSERT"}, /**< insert on PC, help on some Mac keyboards (but
//                                   does send code 73, not 117) */
//{74,"HOME"},
//{75,"PAGEUP"},
//{76,"DELETE"},
//{77,"END"},
//{78,"PAGEDOWN"},
//{79,"RIGHT"},
//{80,"LEFT"},
//{81,"DOWN"},
//{82,"UP"},
//
//{83,"NUMLOCKCLEAR"}, /**< num lock on PC, clear on Mac keyboards
//                                     */
//{84,"KP_DIVIDE"},
//{85,"KP_MULTIPLY"},
//{86,"KP_MINUS"},
//{87,"KP_PLUS"},
//{88,"KP_ENTER"},
//{89,"KP_1"},
//{90,"KP_2"},
//{91,"KP_3"},
//{92,"KP_4"},
//{93,"KP_5"},
//{94,"KP_6"},
//{95,"KP_7"},
//{96,"KP_8"},
//{97,"KP_9"},
//{98,"KP_0"},
//{99,"KP_PERIOD"},
//
//{100,"NONUSBACKSLASH"}, /**< This is the additional key that ISO
//                                        *   keyboards have over ANSI ones,
//                                        *   located between left shift and Z.
//                                        *   Produces GRAVE ACCENT and TILDE in a
//                                        *   US or UK Mac layout, REVERSE SOLIDUS
//                                        *   (backslash) and VERTICAL LINE in a
//                                        *   US or UK Windows layout, and
//                                        *   LESS-THAN SIGN and GREATER-THAN SIGN
//                                        *   in a Swiss German, German, or French
//                                        *   layout. */
//{101,"APPLICATION"}, /**< windows contextual menu, compose */
//{102,"POWER"}, /**< The USB document says this is a status flag,
//                               *   not a physical key - but some Mac keyboards
//                               *   do have a power key. */
//{103,"KP_EQUALS"},
//{104,"F13"},
//{105,"F14"},
//{106,"F15"},
//{107,"F16"},
//{108,"F17"},
//{109,"F18"},
//{110,"F19"},
//{111,"F20"},
//{112,"F21"},
//{113,"F22"},
//{114,"F23"},
//{115,"F24"},
//{116,"EXECUTE"},
//{117,"HELP"},    /**< AL Integrated Help Center */
//{118,"MENU"},    /**< Menu (show menu) */
//{119,"SELECT"},
//{120,"STOP"},    /**< AC Stop */
//{121,"AGAIN"},   /**< AC Redo/Repeat */
//{122,"UNDO"},    /**< AC Undo */
//{123,"CUT"},     /**< AC Cut */
//{124,"COPY"},    /**< AC Copy */
//{125,"PASTE"},   /**< AC Paste */
//{126,"FIND"},    /**< AC Find */
//{127,"MUTE"},
//{128,"VOLUMEUP"},
//{129,"VOLUMEDOWN"},
///* not sure whether there's a reason to enable these */
//{130,"LOCKINGCAPSLOCK"},  
//{131,"LOCKINGNUMLOCK"}, 
//{132,"LOCKINGSCROLLLOCK"}, 
//{133,"KP_COMMA"},
//{134,"KP_EQUALSAS400"},
//
//{135,"INTERNATIONAL1"}, /**< used on Asian keyboards, see
//                                            footnotes in USB doc */
//{136,"INTERNATIONAL2"},
//{137,"INTERNATIONAL3"}, /**< Yen */
//{138,"INTERNATIONAL4"},
//{139,"INTERNATIONAL5"},
//{140,"INTERNATIONAL6"},
//{141,"INTERNATIONAL7"},
//{142,"INTERNATIONAL8"},
//{143,"INTERNATIONAL9"},
//{144,"LANG1"}, /**< Hangul/English toggle */
//{145,"LANG2"}, /**< Hanja conversion */
//{146,"LANG3"}, /**< Katakana */
//{147,"LANG4"}, /**< Hiragana */
//{148,"LANG5"}, /**< Zenkaku/Hankaku */
//{149,"LANG6"}, /**< reserved */
//{150,"LANG7"}, /**< reserved */
//{151,"LANG8"}, /**< reserved */
//{152,"LANG9"}, /**< reserved */
//
//{153,"ALTERASE"},    /**< Erase-Eaze */
//{154,"SYSREQ"},
//{155,"CANCEL"},      /**< AC Cancel */
//{156,"CLEAR"},
//{157,"PRIOR"},
//{158,"RETURN2"},
//{159,"SEPARATOR"},
//{160,"OUT"},
//{161,"OPER"},
//{162,"CLEARAGAIN"},
//{163,"CRSEL"},
//{164,"EXSEL"},
//
//{176,"KP_00"},
//{177,"KP_000"},
//{178,"THOUSANDSSEPARATOR"},
//{179,"DECIMALSEPARATOR"},
//{180,"CURRENCYUNIT"},
//{181,"CURRENCYSUBUNIT"},
//{182,"KP_LEFTPAREN"},
//{183,"KP_RIGHTPAREN"},
//{184,"KP_LEFTBRACE"},
//{185,"KP_RIGHTBRACE"},
//{186,"KP_TAB"},
//{187,"KP_BACKSPACE"},
//{188,"KP_A"},
//{189,"KP_B"},
//{190,"KP_C"},
//{191,"KP_D"},
//{192,"KP_E"},
//{193,"KP_F"},
//{194,"KP_XOR"},
//{195,"KP_POWER"},
//{196,"KP_PERCENT"},
//{197,"KP_LESS"},
//{198,"KP_GREATER"},
//{199,"KP_AMPERSAND"},
//{200,"KP_DBLAMPERSAND"},
//{201,"KP_VERTICALBAR"},
//{202,"KP_DBLVERTICALBAR"},
//{203,"KP_COLON"},
//{204,"KP_HASH"},
//{205,"KP_SPACE"},
//{206,"KP_AT"},
//{207,"KP_EXCLAM"},
//{208,"KP_MEMSTORE"},
//{209,"KP_MEMRECALL"},
//{210,"KP_MEMCLEAR"},
//{211,"KP_MEMADD"},
//{212,"KP_MEMSUBTRACT"},
//{213,"KP_MEMMULTIPLY"},
//{214,"KP_MEMDIVIDE"},
//{215,"KP_PLUSMINUS"},
//{216,"KP_CLEAR"},
//{217,"KP_CLEARENTRY"},
//{218,"KP_BINARY"},
//{219,"KP_OCTAL"},
//{220,"KP_DECIMAL"},
//{221,"KP_HEXADECIMAL"},
//
//{224,"LCTRL"},
//{225,"LSHIFT"},
//{226,"LALT"}, /**< alt, option */
//{227,"LGUI"}, /**< windows, command (apple), meta */
//{228,"RCTRL"},
//{229,"RSHIFT"},
//{230,"RALT"}, /**< alt gr, option */
//{231,"RGUI"}, /**< windows, command (apple), meta */
//
//{257,"MODE"},    /**< I'm not sure if this is really not covered
//                                 *   by any of the above, but since there's a
//                                 *   special SDL_KMOD_MODE for it I'm adding it here
//                                 */
//
//    /* @}, *//* Usage page 0x07 */
//
//    /**
//     *  \name Usage page 0x0C
//     *
//     *  These values are mapped from usage page 0x0C (USB consumer page).
//     *
//     *  There are way more keys in the spec than we can represent in the
//     *  current scancode range, so pick the ones that commonly come up in
//     *  real world usage.
//     */
//    /* @{ */
//
//{258,"SLEEP"},                   /**< Sleep */
//{259,"WAKE"},                    /**< Wake */
//
//{260,"CHANNEL_INCREMENT"},       /**< Channel Increment */
//{261,"CHANNEL_DECREMENT"},       /**< Channel Decrement */
//
//{262,"MEDIA_PLAY"},          /**< Play */
//{263,"MEDIA_PAUSE"},         /**< Pause */
//{264,"MEDIA_RECORD"},        /**< Record */
//{265,"MEDIA_FAST_FORWARD"},  /**< Fast Forward */
//{266,"MEDIA_REWIND"},        /**< Rewind */
//{267,"MEDIA_NEXT_TRACK"},    /**< Next Track */
//{268,"MEDIA_PREVIOUS_TRACK"}, /**< Previous Track */
//{269,"MEDIA_STOP"},          /**< Stop */
//{270,"MEDIA_EJECT"},         /**< Eject */
//{271,"MEDIA_PLAY_PAUSE"},    /**< Play / Pause */
//{272,"MEDIA_SELECT"},        /* Media Select */
//
//{273,"AC_NEW"},              /**< AC New */
//{274,"AC_OPEN"},             /**< AC Open */
//{275,"AC_CLOSE"},            /**< AC Close */
//{276,"AC_EXIT"},             /**< AC Exit */
//{277,"AC_SAVE"},             /**< AC Save */
//{278,"AC_PRINT"},            /**< AC Print */
//{279,"AC_PROPERTIES"},       /**< AC Properties */
//
//{280,"AC_SEARCH"},           /**< AC Search */
//{281,"AC_HOME"},             /**< AC Home */
//{282,"AC_BACK"},             /**< AC Back */
//{283,"AC_FORWARD"},          /**< AC Forward */
//{284,"AC_STOP"},             /**< AC Stop */
//{285,"AC_REFRESH"},          /**< AC Refresh */
//{286,"AC_BOOKMARKS"},        /**< AC Bookmarks */
//
//    /* @}, *//* Usage page 0x0C */
//
//
//    /**
//     *  \name Mobile keys
//     *
//     *  These are values that are often used on mobile phones.
//     */
//    /* @{ */
//
//{287,"SOFTLEFT"}, /**< Usually situated below the display on phones and
//                                      used as a multi-function feature key for selecting
//                                      a software defined function shown on the bottom left
//                                      of the display. */
//{288,"SOFTRIGHT"}, /**< Usually situated below the display on phones and
//                                       used as a multi-function feature key for selecting
//                                       a software defined function shown on the bottom right
//                                       of the display. */
//{289,"CALL"}, /**< Used for accepting phone calls. */
//{290,"ENDCALL"}, /**< Used for rejecting phone calls. */
//
//    /* @}, *//* Mobile keys */
//
//    /* Add any other keys here. */
//
//{400,"RESERVED"},    /**< 400-500 reserved for dynamic keycodes */
//
//    {512,"COUNT"}, /**< not a key, just marks the number of scancodes for array bounds */
//
//};
//int main(){
//    for (int i = 0; i< 512; i++){
//        auto it = map.find(i);
//        if (it == map.end()){
//            std::println("\"N/A\",");
//        }else{
//            std::println("\"{}\",",it->second);
//        }
//    }
//}
