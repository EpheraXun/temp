/* Copyright (C), 2016-2018, Sourcebrella, Inc Ltd - All rights reserved.
* Unauthorized copying, using, modifying of this file, via any medium is
strictly prohibited.
* Proprietary and confidential.

* Author: Dong Dejun(dongdejun@sbrella.com)
* File Description: A tool to parser clear text.
* Creation Date: 2018.10.9
* Modification History:
*/

#ifndef _CLEARTEXT_PARSER_H_
#define _CLEARTEXT_PARSER_H_

#include <string>

#ifdef WIN32
#define WIN_DLLEXPORT __declspec(dllexport)
#else
#define WIN_DLLEXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

namespace cleartextParser {
    bool isRegexMatch(const std::string &PatternStr, const std::string &Str);
// C doesn't identity constructor.
    struct Result {
        double Probability;
        long long Category;
        double Entropy;
    };

// Detect credit card starts with 4 or 6
    WIN_DLLEXPORT bool creditCardCheck(const std::string &Str);

// Detect URL address
    WIN_DLLEXPORT bool urlCheck(const std::string &Str);

// Detect mac address such as "00:0C:29:88:83:1A"
    WIN_DLLEXPORT bool macCheck(const std::string &Str);

// Detect ipV6 such as "2031:0000:1F1F:0000:0000:0100:11A0:ADDF"
    WIN_DLLEXPORT bool ipV6Check(const std::string &Str);

// Detect ipV4 such as "192.168.0.1"
    WIN_DLLEXPORT bool ipV4Check(const std::string &Str);

// Detect common function names start with "set","get","insert", "update" and so
// on.
    WIN_DLLEXPORT bool commonFuncNameCheck(const std::string &Str);

// Detect Email
    WIN_DLLEXPORT bool emailCheck(const std::string &Str);

// Detect QQ number 5~11 numbers
    WIN_DLLEXPORT bool qqNumCheck(const std::string &Str);

// Detect mobile phone number such as "13032918952"
    WIN_DLLEXPORT bool mobilePhoneNumCheck(const std::string &Str);

// Detect telephone number such as "400-6562211" or "0755-2788288"
    WIN_DLLEXPORT bool telePhoneNumCheck(const std::string &Str);

// Detect date such as "2018-10-11" or "2018/10/11" or "2018.10.11"
    WIN_DLLEXPORT bool dateCheck(const std::string &Str);

// Detect date such as "20181011"
    WIN_DLLEXPORT bool dateWithoutSpaceCheck(const std::string &Str);

// Detect floating-point number
    WIN_DLLEXPORT bool floatNumCheck(const std::string &Str);

// Detect Chinese identification card
    WIN_DLLEXPORT bool idCardCheck(const std::string &Str);

// Detect xml file name
    WIN_DLLEXPORT bool xmlFileNameCheck(const std::string &Str);

// Detect Chinese character code
    WIN_DLLEXPORT bool zhCharacterCheck(const std::string &Str);

// Detect postCode
    WIN_DLLEXPORT bool postCodeCheck(const std::string &Str);

// Detect Chinese spell name
    WIN_DLLEXPORT bool chineseSpellNameCheck(const std::string &Str);

// Detect a statement with whitespace.
    WIN_DLLEXPORT bool statementCheck(const std::string &Str);

// Detect a variable name
    WIN_DLLEXPORT bool commonVarNameCheck(const std::string &Str);

// Detect a variable name such as "${password}"
    WIN_DLLEXPORT bool specVarNameCheck(const std::string &Str);

// Detect a weixin number starts with "weixin"
    WIN_DLLEXPORT bool weixinNumCheck(const std::string &Str);

// Detect common linux path
    WIN_DLLEXPORT bool commonLinuxPathCheck(const std::string &Str);

// Detect special comments such as "FIXME", "TODO", "HACK", "BUG"
    WIN_DLLEXPORT bool specCommentCheck(const std::string &Str);

// Reference
// https://github.com/Yelp/detect-secrets/blob/master/detect_secrets/plugins/high_entropy_strings.py
// Returns the entropy of a given string.
// Borrowed from :
// http://blog.dkbza.org/2007/05/scanning-data-for-entropy-anomalies.html.
// \p Str: The word to analyze.
// Returns : double, between 0.0 and 8.0
    WIN_DLLEXPORT double calculateShannonEntropy(const std::string &Str);

// Determine if the variable name \p VarName contains "pwd", "Password" and so
// on.
    WIN_DLLEXPORT double passwordKeyCheck(std::string Str);

// Detect password, and return a struct with two fields:
// Probability: The probability that \p Str is a password
// Category: The categories of \p Str.
// If the return value Result.Probability is more than 0.6,
// we can regard it as a password. This value can be modified.
    WIN_DLLEXPORT struct Result passwordValueCheck(const std::string &Str);

    WIN_DLLEXPORT struct Result passwordValueCheck2(const std::string &Str);

    WIN_DLLEXPORT bool md5Check(const std::string &Str);

    WIN_DLLEXPORT bool sha1Check(const std::string &Str);

    WIN_DLLEXPORT bool sha256Check(const std::string &Str);

    WIN_DLLEXPORT bool sha512Check(const std::string &Str);

    WIN_DLLEXPORT bool base64Check(const std::string &Str);

    WIN_DLLEXPORT bool rsaPublicCheck(const std::string &Str);

    WIN_DLLEXPORT double unknowWordCheck(const std::string &Str);
}

#ifdef __cplusplus
}
#endif

#endif // _CLEARTEXT_PARSER_H_