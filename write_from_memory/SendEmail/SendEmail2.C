// 功能：通过 QQ SMTPS 向用户邮箱发送一封纯文本测试邮件，
//       不读取或附加任何本地文件，并返回 curl 的实际退出码。
// 方法：优先从 SEND_EMAIL_AUTH_CODE 读取 SMTP 授权码；未设置时在 macOS 上
//       从钥匙串读取。程序构造包含 From、To、Subject 和纯文本正文的 RFC 5322
//       邮件，通过标准输入交给 curl，再由 smtp.qq.com:465 加密发送。
// 注意事项：本文件不会保存或输出授权码；默认发件人和收件人可分别通过
//       SEND_EMAIL_SENDER 和 SEND_EMAIL_RECIPIENT 覆盖。curl 返回 0 仅表示
//       SMTP 服务器接受了邮件，不保证收件服务器最终投递到收件箱。

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

std::string TrimWhitespace(const std::string& value)
{
    const std::size_t first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }

    const std::size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string ReadKeychainPassword(const std::string& account,
                                 const std::string& service)
{
#if defined(__APPLE__)
    const std::string command =
        "/usr/bin/security find-generic-password -a " + ShellQuote(account) +
        " -s " + ShellQuote(service) + " -w 2>/dev/null";

    return TrimWhitespace(gSystem->GetFromPipe(command.c_str()).Data());
#else
    static_cast<void>(account);
    static_cast<void>(service);
    return "";
#endif
}

}  // namespace


Int_t SendEmail2()
{
    const std::string sender =
        GetEnvironment("SEND_EMAIL_SENDER", "2310539989@qq.com");

    const std::string recipient =
        GetEnvironment("SEND_EMAIL_RECIPIENT", "yemingxin@sinap.ac.cn");

    std::string authCode =
        GetEnvironment("SEND_EMAIL_AUTH_CODE");

    const std::string keychainService =
        GetEnvironment("SEND_EMAIL_KEYCHAIN_SERVICE",
                       "codex-send-email-qq-smtp");

    const std::string smtpUrl =
        GetEnvironment("SEND_EMAIL_SMTP_URL",
                       "smtps://smtp.qq.com:465");

    const std::string noProxy =
        GetEnvironment("SEND_EMAIL_NO_PROXY",
                       "smtp.qq.com");


    if (authCode.empty()) {
        authCode = ReadKeychainPassword(sender, keychainService);
    }

    if (authCode.empty()) {
        std::cerr
            << "SMTP authorization code is unavailable. Set "
            << "SEND_EMAIL_AUTH_CODE or configure the macOS Keychain item: "
            << keychainService
            << std::endl;

        return 2;
    }


    // 构造纯文本邮件
    const std::string message =
        "From: " + sender + "\r\n"
        "To: " + recipient + "\r\n"
        "Subject: ESPRI SMTP Test Message\r\n"
        "MIME-Version: 1.0\r\n"
        "Content-Type: text/plain; charset=UTF-8\r\n"
        "Content-Transfer-Encoding: 8bit\r\n"
        "\r\n"
        "Hello,\r\n"
        "\r\n"
        "This is a test email sent automatically from a ROOT program.\r\n"
        "\r\n"
        "The purpose of this message is to verify that the SMTP connection\r\n"
        "between the local computer and the QQ mail server is working correctly.\r\n"
        "\r\n"
        "Test information:\r\n"
        "  - SMTP server: smtp.qq.com\r\n"
        "  - Connection type: SMTPS\r\n"
        "  - Port: 465\r\n"
        "  - Authentication: AUTH LOGIN\r\n"
        "  - Message format: plain text\r\n"
        "\r\n"
        "If this email is received successfully, the basic SMTP sending\r\n"
        "function of the ROOT analysis program is working normally.\r\n"
        "\r\n"
        "Best regards,\r\n"
        "Mingxin Ye\r\n";


    std::string command =
        "printf %s " + ShellQuote(message) + " | "
        "curl --silent --show-error --url " +
        ShellQuote(smtpUrl) + " ";

    if (!noProxy.empty()) {
        command +=
            "--noproxy " + ShellQuote(noProxy) + " ";
    }


    command +=
        "--ssl-reqd "
        "--login-options " + ShellQuote("AUTH=LOGIN") + " "
        "--user " + ShellQuote(sender + ":" + authCode) + " "
        "--mail-from " + ShellQuote("<" + sender + ">") + " "
        "--mail-rcpt " + ShellQuote("<" + recipient + ">") + " "
        "--upload-file -";


    const Int_t status =
        gSystem->Exec(command.c_str());

    Int_t curlExitCode = 1;

    if (status >= 0 && WIFEXITED(status)) {
        curlExitCode = WEXITSTATUS(status);
    }


    std::cout
        << "curl exit code = "
        << curlExitCode
        << std::endl;

    return curlExitCode;
}