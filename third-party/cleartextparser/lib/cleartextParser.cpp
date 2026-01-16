/* Copyright (C), 2016-2018, Sourcebrella, Inc Ltd - All rights reserved.
* Unauthorized copying, using, modifying of this file, via any medium is
strictly prohibited.
* Proprietary and confidential.

* Author: Dong Dejun(dongdejun@sbrella.com)
* File Description: A tool to parser clear text.
* Creation Date: 2018.10.9
* Modification History:
*/

#include "cleartextParser.h"
#include <algorithm>
#include <ctype.h>
#include <iostream>
#include <math.h>
#include <string.h>
#include <vector>
#include <set>

#define PCRE_STATIC // static library compiler option

#include "pcre.h"

#define OVECCOUNT 123 /* should be a multiple of 3 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define PRINT_LOG 0

#define FLAG_CREDIT_CARD (1 << 0)
#define FLAG_URL (1 << 1)
#define FLAG_MAC (1 << 2)
#define FLAG_IPV4 (1 << 3)
#define FLAG_IPV6 (1 << 4)
#define FLAG_COMMON_FUNC_NAME (1 << 5)
#define FLAG_QQ_NUM (1 << 6)
#define FLAG_EMAIL (1 << 7)
#define FLAG_MOBILE_PHONE_NUM (1 << 8)
#define FLAG_TELE_PHONE_NUM (1 << 9)
#define FLAG_DATE (1 << 10)
#define FLAG_FLOAT_NUM (1 << 11)
#define FLAG_ID_CARD (1 << 12)
#define FLAG_XML_FILE_NAME (1 << 13)
#define FLAG_ZH_CHARACTER (1 << 14)
#define FLAG_POST_CODE (1 << 15)
#define FLAG_CHINESE_SPELLNAME (1 << 16)
#define FLAG_STATEMENT (1 << 17)
#define FLAG_SPEC_VAR_NAME (1 << 18)

using namespace std;

namespace cleartextParser {
    static std::vector<std::string> PwdDict = {
        #include "password.def"
    };

    static std::vector<std::string> WordVec = {
        #include "word.def"
    };

    static std::vector<std::string> PwdReg = {
        #include "passregex.def"
    };

    static std::vector<std::string> PwdRegBlack = {
        #include "passregblack.def"
    };

    static std::vector<std::string> PwdKeyVec = {
            "pwd", "password", "passwd", "secret", "userpassword", "aeskey", "deskey",
            "privatekey"
    };

    static std::vector<std::string> PwdKeyRegVec = {
            "pwd", "password", "passwd", "secret", "userpassword"
    };

    static std::vector<std::string> UNameKeyVec = {
            "name", "username", "usrname", "compile", "match", "regex", "pattern", "format", "print", "replace"
    };

    // IgnoreCase
    bool isRegexMatch(const std::string &PatternStr, const std::string &Str) {
        pcre *RE;
        const char *Error;
        int ErrOffset;
        int OffsetVec[OVECCOUNT];
        int RC;
        int Options = PCRE_CASELESS;
        const char *Src = Str.c_str();

        if (PRINT_LOG) {
            printf("String : %s\n", Src);
            printf("Pattern: %s\n", PatternStr.c_str());
        }

        RE = pcre_compile(PatternStr.c_str(), Options, &Error, &ErrOffset, NULL);
        if (RE == NULL) {
            if (PRINT_LOG) {
                printf("PCRE compilation failed at offset %d: %s/n", ErrOffset, Error);
            }
            return false;
        }

        // Returns:
        //   > 0 => success; value is the number of elements filled in
        //   = 0 => success, but offsets is not big enough
        //    -1 => failed to match
        //  < -1 => some kind of unexpected problem
        RC = pcre_exec(RE, NULL, Src, (int) strlen(Src), 0, 0, OffsetVec, OVECCOUNT);
        if (RC < 0) {
            if (PRINT_LOG) {
                if (RC == PCRE_ERROR_NOMATCH) {
                    printf("Sorry, no match ...\n");
                } else {
                    printf("Matching error %d\n", RC);
                }
            }

            pcre_free(RE);
            return false;
        }

        if (PRINT_LOG) {
            for (int i = 0; i < RC; i++) {
                const char *substring_start = Src + OffsetVec[2 * i];
                int substring_length = OffsetVec[2 * i + 1] - OffsetVec[2 * i];
                printf("%2d: %.*s\n", i, substring_length, substring_start);
            }
        }

        pcre_free(RE);
        return true;
    }

    bool creditCardCheck(const std::string &Str) {
        if (isRegexMatch("^([46]\\d{12}(?:\\d{3})?)$", Str)) {
            return true;
        }
        return false;
    }

    bool urlCheck(const std::string &Str) {
        if (isRegexMatch("^((https|http|ftp|rtsp|mms)?:\\/\\/)[^\\s]+$", Str)) {
            return true;
        }
        return false;
    }

    bool macCheck(const std::string &Str) {
        if (isRegexMatch("^[0-9a-fA-F]{2}(:[0-9a-fA-F]{2}){5}$", Str)) {
            return true;
        }
        return false;
    }

    bool ipV6Check(const std::string &Str) {
        if (isRegexMatch("^(([\\da-fA-F]{1,4}):){7}([\\da-fA-F]{1,4})$", Str)) {
            return true;
        }
        return false;
    }

    bool ipV4Check(const std::string &Str) {
        if (isRegexMatch("^((25[0-5]|2[0-4]\\d|[01]?\\d\\d?)\\.){3}(25[0-5]|2[0-4]"
                         "\\d|[01]?\\d\\d?)$",
                         Str)) {
            return true;
        }
        return false;
    }

    bool commonFuncNameCheck(const std::string &Str) {
        if (isRegexMatch("^(set|get|insert|update|select|delete|query)(\\w)*$",
                         Str)) {
            return true;
        }
        return false;
    }

    bool qqNumCheck(const std::string &Str) {
        if (isRegexMatch("^[1-9]([0-9]{4,10})$", Str)) {
            return true;
        }
        return false;
    }

    bool emailCheck(const std::string &Str) {
        if (isRegexMatch("^\\w+([-+.]\\w+)*@\\w+([-.]\\w+)*\\.\\w+([-.]\\w+)*$",
                         Str)) {
            return true;
        }
        return false;
    }

    bool mobilePhoneNumCheck(const std::string &Str) {
        if (isRegexMatch("^(\\+\\d{2}-)?(\\d{2,3}-)?([1][3,4,5,7,8][0-9]\\d{8})$",
                         Str)) {
            return true;
        }
        return false;
    }

    bool telePhoneNumCheck(const std::string &Str) {
        if (isRegexMatch("^(\\+\\d{2}-)?0\\d{2,3}-\\d{7,8}$", Str)) {
            return true;
        }
        return false;
    }

    bool dateCheck(const std::string &Str) {
        if (isRegexMatch("^[0-9]{4}(\\-|\\/|.)(((0[13578]|(10|12))(\\-|\\/"
                         "|.)(0[1-9]|[1-2][0-9]|3[0-1]))|(02(\\-|\\/"
                         "|.)(0[1-9]|[1-2][0-9]))|((0[469]|11)(\\-|\\/"
                         "|.)(0[1-9]|[1-2][0-9]|30)))$",
                         Str)) {
            return true;
        }
        return false;
    }

    bool dateWithoutSpaceCheck(const std::string &Str) {
        if (isRegexMatch("^[0-9]{4}(((0[13578]|(10|12))(0[1-9]|[1-2][0-9]|3[0-1]))|("
                         "02(0[1-9]|[1-2][0-9]))|((0[469]|11)(0[1-9]|[1-2][0-9]|30)))"
                         "$",
                         Str)) {
            return true;
        }
        return false;
    }

    bool floatNumCheck(const std::string &Str) {
        if (isRegexMatch("^[-+]?[0-9]*\\.[0-9]+$", Str)) {
            return true;
        }
        return false;
    }

    bool idCardCheck(const std::string &Str) {
        if (isRegexMatch("(^\\d{15}$)|(^\\d{18}$)|(^\\d{17}(\\d|X|x)$)", Str)) {
            return true;
        }
        return false;
    }

    bool xmlFileNameCheck(const std::string &Str) {
        if (isRegexMatch("^([a-zA-Z]+-\?)+[a-zA-Z0-9]+\\.[x|X][m|M][l|L]$", Str)) {
            return true;
        }
        return false;
    }

    bool zhCharacterCheck(const std::string &Str) {
        if (isRegexMatch("^[\\u4e00-\\u9fa5]+$", Str)) {
            return true;
        }
        return false;
    }

    bool postCodeCheck(const std::string &Str) {
        if (isRegexMatch("^[1-9]\\d{5}(?!\\d)$", Str)) {
            return true;
        }
        return false;
    }

    bool chineseSpellNameCheck(const std::string &Str) {
        if (isRegexMatch(
                "(bang|ba[ino]?|beng|be[in]?|bing|bia[no]?|bi[en]?|bu|cang|ca[ino]?|"
                "ceng|ce[in]?|chang|cha[ino]?|cheng|che[n]?|chi|chong|chou|chuang|"
                "chua[in]|chu[ino]?|ci|cong|cou|cuan|cu[ino]?|dang|da[ino]?|deng|de["
                "in]?|dia[no]?|ding|di[ae]?|dong|dou|duan|du[ino]?|fang|fan|fa|feng|"
                "fe[in]{1}|fo[u]?|fu|gang|ga[ino]?|geng|ge[in]?|gong|gou|guang|gua["
                "in]?|gu[ino]?|hang|ha[ino]?|heng|he[in]?|hong|hou|huang|hua[in]?|hu["
                "ino]?|jiang|jia[no]?|jiong|ji[nu]?|juan|ju[en]?|kang|ka[ino]?|keng|"
                "ke[n]?|kong|kou|kuang|kua[in]?|ku[ino]?|lang|la[ino]?|leng|le[i]?|"
                "liang|lia[no]?|ling|li[enu]?|long|lou|luan|lu[no]?|lv[e]?|mang|ma["
                "ino]?|meng|me[in]?|mia[no]?|ming|mi[nu]?|mo[u]?|mu|nang|na[ino]?|"
                "neng|ne[in]?|niang|nia[no]?|ning|ni[enu]?|nong|nou|nuan|nu[on]?|nv["
                "e]?|pang|pa[ino]?|pa|peng|pe[in]?|ping|pia[no]?|pi[en]?|po[u]?|pu|"
                "qiang|qia[no]?|qiong|qing|qi[aenu]?|quan|qu[en]?|rang|ra[no]{1}|"
                "reng|re[n]?|rong|rou|ri|ruan|ru[ino]?|sang|sa[ino]?|seng|se[n]?|"
                "shang|sha[ino]?|sheng|she[in]?|shi|shou|shuang|shua[in]?|shu[ino]?|"
                "si|song|sou|suan|su[ino]?|tang|ta[ino]?|teng|te|ting|ti[e]?|tia[no]?"
                "|tong|tou|tuan|tu[ino]?|wang|wa[ni]?|weng|we[in]{1}|w[ou]{1}|xiang|"
                "xia[no]?|xiong|xing|xi[enu]?|xuan|xu[en]|yang|ya[no]?|ye|ying|yi[n]?"
                "|yong|you|yo|yuan|yu[en]?|zang|za[ino]?|zeng|ze[in]?|zhang|zha[ino]?"
                "|zheng|zhe[in]?|zhi|zhong|zhou|zhuang|zhua[in]?|zhu[ino]?|zi|zong|"
                "zou|zuan|zu[ino]?)",
                Str)) {
            return true;
        }
        return false;
    }

    bool statementCheck(const std::string &Str) {
        if (isRegexMatch("[\\s]+", Str)) {
            return true;
        }
        return false;
    }

    bool commonVarNameCheck(const std::string &Str) {
        if (isRegexMatch("^[_a-zA-Z]\\w*$", Str)) {
            return true;
        }
        return false;
    }

// "${password}" or "${abc}" is a variable name
    bool specVarNameCheck(const std::string &Str) {
        if (isRegexMatch("^\\${[A-Za-z_]+\\w*}$", Str)) {
            return true;
        }
        return false;
    }

    bool weixinNumCheck(const std::string &Str) {
        if (isRegexMatch("^weixin\\w+$", Str)) {
            return true;
        }
        return false;
    }

    bool commonLinuxPathCheck(const std::string &Str) {
        if (isRegexMatch("^(/usr|/bin|/dev|/etc|/home|/lib|/sbin|/tmp|/root|/mnt|/"
                         "proc|/var)(/[\\w-]+)*/*$",
                         Str)) {
            return true;
        }
        return false;
    }

    bool specCommentCheck(const std::string &Str) {
        if (isRegexMatch("^(FIXME|FIX-ME|TODO|TO-DO|HACK|BUG)[\\S\\s]*", Str)) {
            return true;
        }
        return false;
    }

    static std::string initKeyRegStr(const std::vector<std::string> &RegVec) {
        std::string TempleteStr = "^[a-zA-Z_]*(<--KEY WORD-->)\\w*$";
        std::string KeyStr = "";
        for (auto Reg : RegVec) {
            KeyStr.append(Reg);
            KeyStr.append("|");
        }
        if (KeyStr != "") {
            KeyStr = KeyStr.substr(0, KeyStr.length() - 1);
        }
        std::string KeyWord = "<--KEY WORD-->";
        return TempleteStr.replace(TempleteStr.find(KeyWord), KeyWord.length(),
                                   KeyStr);
    }

    double passwordKeyCheck(std::string VarName) {
        transform(VarName.begin(), VarName.end(), VarName.begin(), ::tolower);
        char needToRemove[] = "_-";
        for (unsigned int i = 0; i < strlen(needToRemove); i++) {
            VarName.erase(std::remove(VarName.begin(), VarName.end(), needToRemove[i]), VarName.end());
        }
        if (std::find(PwdKeyVec.begin(), PwdKeyVec.end(), VarName) !=
            PwdKeyVec.end()) {
            return 1.0;
        }

        std::string PwdKeyRegStr = "";
        if (PwdKeyRegStr == "") {
            PwdKeyRegStr = initKeyRegStr(PwdKeyRegVec);
        }

        if (isRegexMatch(PwdKeyRegStr, VarName)) {
            return 0.5;
        }

        if (std::find(UNameKeyVec.begin(), UNameKeyVec.end(), VarName) !=
            UNameKeyVec.end()) {
            return 0;
        }

        std::string UNameKeyRegStr = "";
        if (UNameKeyRegStr == "") {
            UNameKeyRegStr = initKeyRegStr(UNameKeyVec);
        }

        if (isRegexMatch(UNameKeyRegStr, VarName)) {
            return 0.1;
        }

        return 0.2;
    }

// H(X) = -Σp(x) * log2(p(x)),(x∈X)
    double calculateShannonEntropy(const std::string &Str) {
        int Count[256] = {0};
        int TotalNum = 0;
        double Ret = 0.0;

        for (int I = 0; I < Str.length(); ++I) {
            if (isgraph(Str[I])) {
                Count[Str[I]]++;
                TotalNum++;
            }
        }

        for (int I = 0; I < 256; ++I) {
            double P = Count[I] * 1.0 / TotalNum;
            // P != 0
            if (fabs(P - 0) > 1e-6) {
                Ret += -(P * (log(P) / log(2)));
            }
        }

        return Ret;
    }

#define CHECK_SEMANTIC(FuncName, Str, Category, Flag)                          \
  \
if(FuncName(Str)) {                                                            \
    Category |= Flag;                                                          \
  \
}

    long long checkSpecSemantic(const std::string &Str) {
        long long Category = 0;

        CHECK_SEMANTIC(creditCardCheck, Str, Category, FLAG_CREDIT_CARD);
        CHECK_SEMANTIC(urlCheck, Str, Category, FLAG_URL);
        CHECK_SEMANTIC(macCheck, Str, Category, FLAG_MAC);
        CHECK_SEMANTIC(ipV6Check, Str, Category, FLAG_IPV6);
        CHECK_SEMANTIC(ipV4Check, Str, Category, FLAG_IPV4);
        CHECK_SEMANTIC(commonFuncNameCheck, Str, Category, FLAG_COMMON_FUNC_NAME);
        CHECK_SEMANTIC(emailCheck, Str, Category, FLAG_EMAIL);
        // CHECK_SEMANTIC(qqNumCheck, Str, Category, FLAG_QQ_NUM);
        CHECK_SEMANTIC(mobilePhoneNumCheck, Str, Category, FLAG_MOBILE_PHONE_NUM);
        CHECK_SEMANTIC(telePhoneNumCheck, Str, Category, FLAG_TELE_PHONE_NUM);
        CHECK_SEMANTIC(dateCheck, Str, Category, FLAG_DATE);
        CHECK_SEMANTIC(floatNumCheck, Str, Category, FLAG_FLOAT_NUM);
//        CHECK_SEMANTIC(idCardCheck, Str, Category, FLAG_ID_CARD);
        CHECK_SEMANTIC(xmlFileNameCheck, Str, Category, FLAG_XML_FILE_NAME);
        CHECK_SEMANTIC(postCodeCheck, Str, Category, FLAG_POST_CODE);
        CHECK_SEMANTIC(zhCharacterCheck, Str, Category, FLAG_ZH_CHARACTER);
        CHECK_SEMANTIC(specVarNameCheck, Str, Category, FLAG_SPEC_VAR_NAME);
        // CHECK_SEMANTIC(chineseSpellNameCheck, Str, Category,
        // FLAG_CHINESE_SPELLNAME);
        // CHECK_SEMANTIC(statementCheck, Str, Category, FLAG_STATEMENT);

        return Category;
    }

// Some special strings have a high probability of being a password
    static bool isSpecPassword(const std::string &Str) {
        return dateWithoutSpaceCheck(Str);
    }

    struct Result passwordValueCheck(const std::string &Str) {
        Result Ret;
        Ret.Category = 0;
        Ret.Probability = 0;
        Ret.Entropy = 0;

        // If Str has whitespace character, we think it must not be a password.
        if (statementCheck(Str)) {
            Ret.Category |= FLAG_STATEMENT;
            return Ret;
        }
        // If Str in password white list, we think it may be a password.
        if ((std::find(PwdDict.begin(), PwdDict.end(), Str) != PwdDict.end()) ||
            isSpecPassword(Str)) {
            Ret.Probability += 20;
        }

        Ret.Category = checkSpecSemantic(Str);
        if (Ret.Category > 1) {
            // Str has some special semantic
            Ret.Probability -= 15;
        }

        Ret.Entropy = calculateShannonEntropy(Str);
        if (Ret.Entropy > 4.5) {
            Ret.Probability += 30;
        } else if (Ret.Entropy > 3.5) { // (25, 30]
            Ret.Probability += 25;
            Ret.Probability += (Ret.Entropy - 3.5) * 5;
        } else if (Ret.Entropy > 3.0) { // (20, 25]
            Ret.Probability += 20;
            Ret.Probability += (Ret.Entropy - 3.0) * 10;
        } else if (Ret.Entropy > 2.0) { // (5, 20]
            Ret.Probability += 5;
            Ret.Probability += (Ret.Entropy - 2.0) * 15;
        } else if (Ret.Entropy > 1.5) { // (0, 5]
            Ret.Probability += (Ret.Entropy - 1.5) * 10;
        }

        Ret.Probability = Ret.Probability > 0 ? Ret.Probability / 50 : 0;
        return Ret;
    }

    struct Result passwordValueCheck2(const std::string &Str) {
        Result ret = {0, 0, 0};

        if (std::find_if(PwdReg.begin(), PwdReg.end(), [&](std::string& pwdRegex) {
            return isRegexMatch(pwdRegex, Str);
        }) != PwdReg.end()) {
            if (std::find_if(PwdRegBlack.begin(), PwdRegBlack.end(), [&] (std::string& word) {
                return Str.find(word) != std::string::npos;
            }) == PwdRegBlack.end()) {
                ret.Probability = 1;
            }
        }
        ret.Category = checkSpecSemantic(Str);
        return ret;
    }

    bool md5Check(const std::string &Str) {
        if (isRegexMatch("^[0-9A-Fa-f]{32}$", Str)) {
            return true;
        }
        return false;
    }

    bool sha1Check(const std::string &Str) {
        if (isRegexMatch("^[0-9A-Fa-f]{40}$", Str)) {
            return true;
        }
        return false;
    }

    bool sha256Check(const std::string &Str) {
        if (isRegexMatch("^[0-9A-Fa-f]{64}$", Str)) {
            return true;
        }
        return false;
    }

    bool sha512Check(const std::string &Str) {
        if (isRegexMatch("^[0-9A-Fa-f]{128}$", Str)) {
            return true;
        }
        return false;
    }

    bool base64Check(const std::string &Str) {
        if (isRegexMatch("^(?:[A-Za-z0-9+/]{4})*(?:[A-Za-z0-9+/]{2}==|[A-Za-z0-9+/]{3}=)?$", Str)) {
            return true;
        }
        return false;
    }

    bool rsaPublicCheck(const std::string &Str) {
        if (isRegexMatch("AAAAB3NzaC1yc2EA[0-9a-zA-Z=/+]{356}", Str)) {
            return true;
        }
        return false;
    }

    struct TypeRes {
        int count;
        int maxInARow;
        bool inARow;
    };

    enum TypeEnum {
        Lower = 0,
        Upper,
        Number,
        Other,
        None
    };

    static std::vector<char> charVec = {
            '\\', ' ', '[', ']'
    };

    TypeEnum checkCharType(char c, int& perSign) {
        if (c >= 'a' && c <= 'z') {
            return Lower;
        } else if (c >= 'A' && c <= 'Z') {
            return Upper;
        } else if (c >= '0' && c <= '9') {
            return Number;
        } else if (c >= 0 && c <= 127) {
            if (std::find(charVec.begin(), charVec.end(), c) != charVec.end()) {
                return None;
            }
            if (c == '%') {
                perSign++;
            }
            return Other;
        } else {
            return None;
        }
    }

    bool wordFilter(const std::string &Str) {
        std::string data;
        data.resize(Str.length());
        std::transform(Str.begin(), Str.end(), data.begin(), [] (char in) {
            if ((in >= 'A') && (in <= 'Z')) {
                return (char)(in - ('Z' - 'z'));
            }
            return in;
        });
        auto word = std::find_if(WordVec.begin(), WordVec.end(), [&] (std::string& word) {
            if (word.size() > 4) {
                return data.find(word) != std::string::npos;
            } else {
                if (Str.find(word) != std::string::npos) {
                    return true;
                }
                std::string upperWord;
                upperWord.resize(word.size());
                std::transform(word.begin(), word.end(), upperWord.begin(), [] (char in) {
                    if ((in >= 'a') && (in <= 'z')) {
                        return (char) (in - ('z' - 'Z'));
                    }
                });
                if (Str.find(upperWord) != std::string::npos) {
                    return true;
                }
                std::string firstUpper(word);
                std::transform(firstUpper.begin(), firstUpper.begin() + 1, firstUpper.begin(), [] (char in) {
                    if ((in >= 'a') && (in <= 'z')) {
                      return (char) (in - ('z' - 'Z'));
                    }
                });
                if (Str.find(firstUpper) != std::string::npos) {
                    return true;
                }
                return false;
            }
        });
        if (word != WordVec.end()) {
            return true;
        }
        return false;
    }

    static std::set<std::string> NumSet = {
        "131071", "131072", "262143", "262144", "524287", "524288",
        "1048575", "1048576", "2097151", "2097152", "4194303", "4194304",
        "8388607", "8388608", "16777215", "16777216", "33554431", "33554432",
        "67108863", "67108864", "134217727", "134217728", "268435455", "268435456",
        "536870911", "536870912", "1073741823", "1073741824", "2147483647", "2147483648",
        "4294967295", "4294967296", "9223372036854775807", "9223372036854775808",
        "18446744073709551615", "18446744073709551616"
    };

    double magicNumberCheck(const std::string &Str) {

        if ((Str.length() <= 5) || (NumSet.find(Str) != NumSet.end())) {
            return false;
        }

        int couts[10] = {0};
        for (char c : Str) {
            couts[c - '0']++;
        }
        if (couts[0] > (Str.length() / 2)) {
            return false;
        }
        int maxCount = 0;
        for (int i = 1; i < 10; i++) {
            if (couts[i] > maxCount) {
                maxCount = couts[i];
            }
        }
        if (maxCount == Str.length()) {
            return false;
        }
        return true;
    }

    double unknowWordCheck(const std::string &Str) {

        if (wordFilter(Str)) {
            return false;
        }
        TypeRes res[4] = {TypeRes{0, 0, false}};
        TypeEnum lastChar = None;
        int currentRowCount = 0;
        int percentSign = 0;
        for (char c : Str) {
            TypeEnum cur = checkCharType(c, percentSign);
            if (cur == None) {
                return false;
            }
            res[cur].count++;
            if (cur == lastChar) {
                res[cur].inARow = true;
                currentRowCount++;
            } else {
                if (res[lastChar].maxInARow < currentRowCount) {
                    res[lastChar].maxInARow = currentRowCount;
                }
                currentRowCount = 1;
            }
            lastChar = cur;
        }
        if (res[lastChar].maxInARow < currentRowCount) {
            res[lastChar].maxInARow = currentRowCount;
        }
        if (percentSign >= (Str.length() / 6 + 1)) {
            return false;
        }
        if (res[Number].count == Str.length()) {
            return magicNumberCheck(Str);
        }
        if (Str.length() < 16) {
            return false;
        }
        int inRowTypeCount = 0;
        int typeMaxCount = 0;
        int maxRowTypeRes = 0;
        for (TypeRes tr : res) {
            if ((tr.maxInARow == tr.count) && (tr.maxInARow > 2)) {
                maxRowTypeRes++;
            }
            if (tr.count > typeMaxCount) {
                typeMaxCount = tr.count;
            }
            if (tr.inARow) {
                inRowTypeCount++;
            }
        }
        return (typeMaxCount < (Str.length() * 0.75)) && (inRowTypeCount > 1) && (maxRowTypeRes == 0);
    }
}