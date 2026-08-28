// 功能：通过 QQ SMTPS 向用户邮箱发送一封主题和正文均为 Hello 的测试邮件，
//       不读取或附加任何本地文件，并返回 curl 的实际退出码。
// 方法：SMTP 授权码由用户手动填写在 SendEmail3 函数内的 authCode 空字符串中。
//       程序构造包含 From、To、Subject 和纯文本正文的 RFC 5322 邮件，通过
//       标准输入交给 curl，再由 smtp.qq.com:465 加密发送。
// 注意事项：授权码填入源码后会以明文保存，禁止提交或分享本文件；默认发件人和
//       收件人可由 SEND_EMAIL_SENDER、SEND_EMAIL_RECIPIENT 覆盖。curl 返回
//       0 仅表示 SMTP 服务器接受了邮件，不保证最终投递到收件箱。

#include <iostream>
#include <string>
#include <sys/wait.h>

#include "TSystem.h"

namespace {

std::string GetEnvironment(const char* name, const char* fallback = "")
{
    const char* value = gSystem->Getenv(name);
    return (value && value[0] != '\0') ? value : fallback;
}

std::string ShellQuote(const std::string& value)
{
    std::string quoted = "'";
    for (const char character : value) {
        if (character == '\'') {
            quoted += "'\\''";
        } else {
            quoted += character;
        }
    }
    quoted += "'";
    return quoted;
}

}  // namespace

Int_t SendEmail3()
{
    const std::string sender =
        GetEnvironment("SEND_EMAIL_SENDER", "2310539989@qq.com");
    const std::string recipient =
        GetEnvironment("SEND_EMAIL_RECIPIENT", "2310539989@qq.com");
    const std::string authCode = "mfdvcttlxwlfebcd";  // 在双引号内手动填写 QQ SMTP 授权码
    const std::string smtpUrl =
        GetEnvironment("SEND_EMAIL_SMTP_URL", "smtps://smtp.qq.com:465");
    const std::string noProxy =
        GetEnvironment("SEND_EMAIL_NO_PROXY", "smtp.qq.com");

    if (authCode.empty()) {
        std::cerr << "SMTP authorization code is empty. Fill authCode in "
                  << "SendEmail3.C before running." << std::endl;
        return 2;
    }

    const std::string message =
        "From: " + sender + "\r\n"
        "To: " + recipient + "\r\n"
        "Subject: Hello\r\n"
        "MIME-Version: 1.0\r\n"
        "Content-Type: text/plain; charset=UTF-8\r\n"
        "\r\n"
        "Hello\r\n";

    std::string command = "printf %s " + ShellQuote(message) + " | "
                          "curl --silent --show-error --url " +
                          ShellQuote(smtpUrl) + " ";
    if (!noProxy.empty()) {
        command += "--noproxy " + ShellQuote(noProxy) + " ";
    }
    command +=
        "--ssl-reqd "
        "--login-options " + ShellQuote("AUTH=LOGIN") + " "
        "--user " + ShellQuote(sender + ":" + authCode) + " "
        "--mail-from " + ShellQuote("<" + sender + ">") + " "
        "--mail-rcpt " + ShellQuote("<" + recipient + ">") + " "
        "--upload-file -";

    const Int_t status = gSystem->Exec(command.c_str());
    Int_t curlExitCode = 1;
    if (status >= 0 && WIFEXITED(status)) {
        curlExitCode = WEXITSTATUS(status);
    }

    std::cout << "curl exit code = " << curlExitCode << std::endl;
    return curlExitCode;
}
