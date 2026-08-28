#include "TSystem.h"
#include <iostream>
#include <string>
#include <sys/wait.h>
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/Timer/Timer.h"
#include "/Users/yemingxin/anaroot/output/Analysis/MyToolsFunction/Timer/Timer.cc"

//其中 \n 表示换行。邮件标题可通过 subject 变量修改

void SendMail()
{
    Timer timer;
    std::string sender    = "2310539989@qq.com";
    std::string recipient = "yemingxin@sinap.ac.cn";
    std::string code      = "mfdvcttlxwlfebcd";
    std::string png       = "/Users/yemingxin/Desktop/截屏2026-07-26 22.05.39.png";
    std::string pdf       = "/Users/yemingxin/Downloads/IWND2026日程安排.pdf";
    std::string subject   = "ROOT Test";

    //正文内容
    std::string MainText =
        "Hello,\n"
        "\n"
        "This is an email sent by a ROOT macro.\n"
        "The image is included as an attachment.\n"
        "\n"
        "Best regards,\n"
        "Mingxin Ye";

    std::string cmd =
        "curl --url 'smtps://smtp.qq.com:465' "
        "--noproxy 'smtp.qq.com' "
        "--login-options 'AUTH=LOGIN' "
        "--user '" + sender + ":" + code + "' "
        "--mail-from '<" + sender + ">' "
        "--mail-rcpt '<" + recipient + ">' "
        "-H 'From: " + sender + "' "
        "-H 'To: " + recipient + "' "
        "-H 'Subject: " + subject + "' "
        "-F '=" + MainText + ";type=text/plain;charset=utf-8' "
        "-F '=@\"" + png + "\";type=image/png;encoder=base64' "
        "-F '=@\"" + pdf + "\";type=application/pdf;encoder=base64'";

    int status = gSystem->Exec(cmd.c_str());
    int curlExitCode = WEXITSTATUS(status);
    std::cout << "curl exit code = " << curlExitCode << std::endl;
}
