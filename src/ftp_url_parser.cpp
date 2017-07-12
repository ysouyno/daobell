#include "ftp_url_parser.h"

int ftp_url_parser::parse()
{
  if (url_.empty()) {
    log_e("url is empty\n");
    return -1;
  }

  const char *ftp_protocol = "ftp://";
  const char *protocol_delimiter = "://";

  // check if ftp protocol
  if (std::string::npos == url_.find(ftp_protocol)) {
    log_e("url is not ftp\n");
    return -1;
  }

  // TODO: if filename contain '@', parse url will occur errors
  // if user is anonymous and '@' is found, indicating after domain
  // name must have '/', like "ftp://127.0.0.1/@filename"
  size_t pos1 = url_.find_first_of(protocol_delimiter);
  size_t pos2 = url_.find_first_of('@');
  std::string domain_port;

  if (std::string::npos != pos1 && std::string::npos != pos2) {
    log_t("ftp user mode\n");
    std::string user_pass = url_.substr(pos1 + strlen(protocol_delimiter),
                                        pos2 - pos1 - strlen(protocol_delimiter));
    size_t pos = user_pass.find_first_of(':');
    if (std::string::npos == pos) {
      log_e("parse user and password from url error\n");
      return -1;
    }

    user_ = user_pass.substr(0, pos);
    pass_ = user_pass.substr(pos + 1, user_pass.size() - pos - 1);

    domain_port = url_.substr(pos2 + 1, url_.size() - pos - 1);
  }
  else {
    log_t("ftp anonymous mode\n");
    user_ = "anonymous";
    pass_ = "anonymous";

    domain_port = url_.substr(pos1 + strlen(protocol_delimiter),
                              url_.size() - pos1 - strlen(protocol_delimiter));
  }

  // parse domain and port
  size_t pos = domain_port.find_first_of('/');
  if (std::string::npos == pos) {
    // just have domain or port, like "127.0.0.1:21" or "127.0.0.1"
    log_t("not specify file to download in url\n");
    size_t pos_temp = domain_port.find_first_of(':');
    if (std::string::npos == pos_temp) {
      domain_ = domain_port.substr(0, pos_temp);
      port_ = FTP_DEFAULT_PORT;
    }
    else {
      domain_ = domain_port.substr(0, pos_temp);
      port_ = domain_port.substr(pos_temp + 1, domain_port.size());
    }

    path_ = "";
    file_ = "";
  }
  else {
    // maybe just have domain and port with '/' in the end
    // like "127.0.0.1:21/" or "127.0.0.1:21/test/test/txt"
    log_t("specify file to download in url\n");
    size_t pos_temp = domain_port.find_first_of(':');
    if (std::string::npos == pos_temp) {
      domain_ = domain_port.substr(0, pos); // note it's pos
      port_ = FTP_DEFAULT_PORT;
    }
    else {
      domain_ = domain_port.substr(0, pos_temp);
      port_ = domain_port.substr(pos_temp + 1, pos - pos_temp - 1);
    }

    // parse path and file
    std::string path_file = domain_port.substr(pos, domain_port.size() - pos);
    pos_temp = path_file.find_last_of('/');
    if (0 == pos_temp) {
      log_t("file is located in root directory\n");
      path_ = "/";
      file_ = path_file.substr(1, path_file.size() - 1);
    }
    else {
      log_t("file is located in sub directory\n");
      path_ = path_file.substr(0, pos_temp);
      file_ = path_file.substr(pos_temp + 1, path_file.size() - pos_temp - 1);
    }
  }

  return 0;
}

void ftp_url_parser::print_all()
{
  std::cout << "   url_: " << url_ << std::endl;
  std::cout << "  user_: " << user_ << std::endl;
  std::cout << "  pass_: " << pass_ << std::endl;
  std::cout << "domain_: " << domain_ << std::endl;
  std::cout << "  port_: " << port_ << std::endl;
  std::cout << "  path_: " << path_ << std::endl;
  std::cout << "  file_: " << file_ << std::endl;
}
