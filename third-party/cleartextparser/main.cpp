/* Copyright (C), 2016-2018, Sourcebrella, Inc Ltd - All rights reserved.
* Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
* Proprietary and confidential.

* Author: Dong Dejun(dongdejun@sbrella.com)
* File Description: Testing.
* Creation Date: 2018.10.9
* Modification History:
*/


#include "include/cleartextParser.h"
#include <iostream>
#include <cmath>

using namespace std;
using namespace cleartextParser;

static bool TestRegex(bool(*CheckFunc)(const std::string &Str), const std::string& GoodStr, const std::string& BadStr) {
    return CheckFunc(GoodStr) && !CheckFunc(BadStr);
}


#define TEST(FuncName, GoodStr, BadStr)        \
if (TestRegex(FuncName, GoodStr, BadStr)) {    \
    cout << #FuncName << " ok\n";              \
}                                              \
else {                                         \
    cout << #FuncName << " failed!!\n";        \
}

void testAll() {
    TEST(creditCardCheck, "6214832204027532", "1214832204027534");
    TEST(urlCheck, "https://msdn.microsoft.com/zh-tw/library/3y1sfaz2.aspx", "https://msdn.microsoft.co m");
    TEST(macCheck, "00:0C:29:E3:5F:39", "00:0C:29:E3:ZF:39");
    TEST(ipV6Check, "2031:0000:1F1F:0000:0000:0100:11A0:ADDF", "ZZ31:0000:1F1F:0000:0000:0100:11A0:ADDF");
    TEST(ipV4Check, "192.168.0.1", "192.168.0.987");
    TEST(commonFuncNameCheck, "getPwd", "eatPwd");
    TEST(emailCheck, "sbrella@163.com", "sbrella@16 3.com");
    TEST(qqNumCheck, "110000000", "1010101a");
    TEST(mobilePhoneNumCheck, "13032918952", "23032918952");
    TEST(telePhoneNumCheck, "0755-2788288", "0755-278828820");
    TEST(dateCheck, "2018-10-11", "2018/13/11");
    TEST(floatNumCheck, "+3.14", "314");
    TEST(idCardCheck, "37052219910221344X", "370522199102213X14");
    TEST(xmlFileNameCheck, "pom.xml", "pom.txt");
    // TEST(zhCharacterCheck, "哈哈", "haha");
    TEST(postCodeCheck, "257445", "2574452");
    TEST(statementCheck, " helloworld", "helloworld");
    TEST(specVarNameCheck, "${password}", "${password");
    TEST(md5Check, "5B1B718838692C401CAD3AE52755299B", "5B1B718838692C401CAD3LG52755299B")
    TEST(sha1Check, "f00ba57f211c2664d639cc12ecee79912725f3ed", "f00ba57f211c2664d639cc12ecee7991272ooooo")
    TEST(sha256Check, "61b190f9d91e433bc6cc3d26426df472c5af2ef27053ba10b3d469f568fc25c5", "61b190f9d91e433bc6cc3d26426df472c5af2ef27053ba10b3d469f568fc25cp")
    TEST(sha512Check, "09994407903207ff08e35d2a98749d20316b7e659f293cd517624b7dc29ed8de7f2"
                      "1cbda26150189443b0a41c29ef06dc48a233051481fc48527e6a58b2dae69",
                      "09994407903207ff08e35d2a98749d20316b7e659f293cd517624b7dc29ed8"
                      "de7f21cbda26150189443b0a41c29ef06dc48a233051481fc48527e6a58b2dppll")
    TEST(base64Check, "YWRld2ZzZzIxIDQxMmVmc3IzMTMzMTIg", "YWRld2ZzZzIxIDQxMmVmc3IzMTMzMTIg=")
    TEST(rsaPublicCheck, "AAAAB3NzaC1yc2EAAAADAQABAAABAQCs/HYoT2NmSIgCiE/qCjdNM5BM7O1avuspE2PrJSxV2cGkUIiyrJJzGxJ8bgFVqZGs"
                         "uOiTX3JVEF4fotk5CQdUBkCCg+tiw16IPurCXubnqRa2NxHG123kSZJITKMNA1PdFul/KU1neOqvG1ml3QVqzXwwuWwiprLSm"
                         "zJAF37jjh7UvnRvDPa/wUcpAyLNym+k0n8iyGPqNm79tELcilx/dRN+kZYt9vnJmogWi7E7FLqg4o7GLATXf8toDQz3WGzMDQ"
                         "ystTtDwHghTYoftdN2OsnORaznS0oyvWl9DIZ35nyARxJX55wzAh9d2Ng8k8k7HV3vDFmMni1wtKEvEpi9", "ahnguivrhbw")
}

int main() {
    std::cout << passwordKeyCheck("username") << "\n";
    std::cout << passwordKeyCheck("name123") << "\n";
    std::cout << passwordKeyCheck("user") << "\n";
    std::cout << passwordKeyCheck("PassWord") << "\n";
    std::cout << passwordKeyCheck("pwd123") << "\n";
    std::cout << passwordValueCheck2("user32").Probability << "\n";
    std::cout << passwordValueCheck2("user33").Probability << "\n";

    commonLinuxPathCheck("/usr/bin");
    commonLinuxPathCheck("/usr/bi n");
    commonLinuxPathCheck("/root/bin/");
    specCommentCheck("Fixme:");
    specCommentCheck("Todo:dfs  da");
    specCommentCheck("To-do");
    specVarNameCheck("passwordValueCheck");

    std::cout << passwordValueCheck("adminuser123").Probability << "\n";
    std::cout << passwordValueCheck("password").Probability << "\n";
    std::cout << passwordValueCheck("hahaha23").Probability << "\n";
    std::cout << passwordValueCheck("asdfdsfdsf").Probability << "\n";

    // return 0
    std::cout << passwordValueCheck2("user:").Probability << "\n";
    std::cout << passwordValueCheck2("super()").Probability << "\n";
    std::cout << passwordValueCheck2("password_").Probability << "\n";
    // return 1
    std::cout << passwordValueCheck2("qwer1234").Probability << "\n";
    std::cout << passwordValueCheck2("password!@#").Probability << "\n";

    int count = 0;
    int count2 = 0;
    //TEST(chineseSpellNameCheck, "dongdejun", "dongdejun123");
    std::cout << calculateShannonEntropy("administrator") << "\n";
    std::cout << calculateShannonEntropy("admin") << "\n";
    std::cout << calculateShannonEntropy("password") << "\n";
    // return 0
    std::cout << unknowWordCheck("applicationLaunchScheme") << "\n";
    std::cout << unknowWordCheck("/swan/getAppInfoSync") << "\n";
    std::cout << unknowWordCheck("(Ljava/lang/Object;)Z") << "\n";
    std::cout << unknowWordCheck("18446744073709551615") << "\n";
    std::cout << unknowWordCheck("\\dsui1238su3hed85hnfi&&46ks783*(") << "\n";
    std::cout << unknowWordCheck("&ofl=%s|%d|%f|%f|%d") << "\n";
    std::cout << unknowWordCheck("&ver=%s&cuid=%s&prod=%s:%s&sdk=%.2f&mb=%s&os=A%s") << "\n";
    std::cout << unknowWordCheck("u3hs73rhs9yg7923agr8dffu7817@78h.zip") << "\n";
    std::cout << unknowWordCheck("网络加载失败----fhdeuiw378fgh9384yt8yhyhdz990184dd") << "\n";
    std::cout << unknowWordCheck("2m9qSng7R53painXw14") << "\n";
    std::cout << unknowWordCheck("2m9qSng7R53PainXw14") << "\n";
    std::cout << unknowWordCheck("2m9qSng7R53PAINxw14") << "\n";
    // return 1
    std::cout << unknowWordCheck("2m9qSng7R53paInXw14") << "\n";
    std::cout << unknowWordCheck("FLGQGZKwhQpXuUeRNGoVBuLXTeCrOmpC") << "\n";
    std::cout << unknowWordCheck("226573481923844737823748265") << "\n";
    std::cout << unknowWordCheck("vuddi\1nuvj22\2nv3472###72") << "\n";
    std::cout << unknowWordCheck("281736455") << "\n";
    std::cout << passwordKeyCheck("privateKey") << "\n";
    std::cout << passwordKeyCheck("Private_Key") << "\n";
    testAll();
    getchar();
    return 0;
}