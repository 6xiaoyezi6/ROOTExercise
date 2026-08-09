#include "TSystem.h"
#include "TString.h"
#include <iostream>
#include <sys/wait.h>


//其中 \n 表示换行。邮件标题可通过 subject 变量修改

void SendMail()
{
    TString sender    = "2310539989@qq.com";
    TString recipient = "yemingxin@sinap.ac.cn";
    TString code      = "mfdvcttlxwlfebcd";
    TString png       = "/Users/yemingxin/Desktop/截屏2026-07-26 22.05.39.png";
    TString subject   = "ROOT Test";

    TString body =
        "Hello,\n"
        "\n"
        "This is an email sent by a ROOT macro.\n"
        "The image is included as an attachment.\n"
        "\n"
        "Best regards,\n"
        "Yemingxin";

    TString cmd = Form(
        "curl --url 'smtps://smtp.qq.com:465' "
        "--noproxy 'smtp.qq.com' "
        "--login-options 'AUTH=LOGIN' "
        "--user '%s:%s' "
        "--mail-from '<%s>' "
        "--mail-rcpt '<%s>' "
        "-H 'From: %s' "
        "-H 'To: %s' "
        "-H 'Subject: %s' "
        "-F '=%s;type=text/plain;charset=utf-8' "
        "-F '=@\"%s\";type=image/png;encoder=base64'",
        sender.Data(), code.Data(),
        sender.Data(), recipient.Data(),
        sender.Data(), recipient.Data(),
        subject.Data(), body.Data(), png.Data()
    );

    int status = gSystem->Exec(cmd.Data());
    int curlExitCode = WEXITSTATUS(status);
    std::cout << "curl exit code = " << curlExitCode << std::endl;
}
