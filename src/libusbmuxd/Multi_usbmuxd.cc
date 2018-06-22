#include "Multi_usbmuxd.h"
#include "usbmuxd.h"
#include "usbmuxd-proto.h"
#include "sock_stuff.h"
#include "Multi_iproxy.h"
#if (!defined(WIN32)&&!defined(__CYGWIN__))
#include "usbmuxd_mod/src/main.h"
#endif

namespace usbmuxd{
  //project usbmuxd_mod
  //sudo ./usbmuxd --user usbmux --systemd -sp /var/run/usbmuxdxx -spid /var/run/usbmuxdxx.pid
  static void *LocalListenThread(void *arg){
#if defined(WIN32) || defined(__CYGWIN__)
    UsbmuxdDevice* args = reinterpret_cast<UsbmuxdDevice*>(arg);
    const std::string& udid = args->udid();
    CreateListen(udid.c_str(), args->local_port(), args->remote_port());
#else
    char current_execute_path[PATH_MAX] = { 0 };
    int n = readlink("/proc/self/exe", current_execute_path, PATH_MAX);
    char argv[][] = { current_execute_path, 
                      "--user","usbmux",
                      "--systemd",
                      "-sp",args->unix_socket_path().c_str(),
                      "-spid",args->unix_socket_path_lockfile().c_str(),NULL 
                    };
    usbmuxd_main(8, argv);
    //system("sudo /usr/local/sbin/usbmuxd --user usbmux --systemd -sp /var/run/usbmuxdxx -spid /var/run/usbmuxdxx.pid")
#endif
    return nullptr;
  }
  UsbmuxdDevice::UsbmuxdDevice() :usbmuxd_device_info_t_dummy_(nullptr){
    udid_.resize(0);
    local_port_ = 0;
    remote_port_ = 0;
    handle_ = 0;
    product_id_ = 0;
    sock_sfd_ = 0;
    usbmuxd_device_info_t_dummy_ = nullptr;
  }
  void UsbmuxdDevice::SetUdid(const char* udid){
    if (udid&&udid[0]){
      udid_ = udid;
    }
  }
  void UsbmuxdDevice::SetLocalPort(std::uint32_t local_port){
    local_port_ = local_port;
  }
  void UsbmuxdDevice::SetRemotePort(std::uint32_t remote_port){
    remote_port_ = remote_port;
  }
  void UsbmuxdDevice::SetLinuxSocketPath(const char* socket_path) {
    if (socket_path&&socket_path[0]){
      unix_socket_path_ = socket_path;
      std::string pppp = socket_path;
      pppp.append(".pid");
      SetLinuxSocketPathLockFile(socket_path);
    }
  }
  void UsbmuxdDevice::SetLinuxSocketPathLockFile(const char* socket_path_lock_file) {
    if (socket_path_lock_file&&socket_path_lock_file[0]) {
      unix_socket_path_lockfile_ = socket_path_lock_file;
    }
  }
  void UsbmuxdDevice::SetHandle(int handle){
    handle_ = handle;
  }
  void UsbmuxdDevice::SetProductId(int product_id){
    product_id_ = product_id;
  }
  void UsbmuxdDevice::SetSockSFD(int sock_sfd){
    sock_sfd_ = product_id_;
  }
  bool UsbmuxdDevice::SetUsbmuxdDeviceinfo(){
#if defined(WIN32) || defined(__CYGWIN__)
    HANDLE thread_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)LocalListenThread, this, 0, NULL);
    WaitForSingleObject(thread_handle, 200);
    CloseHandle(thread_handle);
#else
    pthread_t acceptor = nullptr;
    pthread_create(&acceptor, NULL, LocalListenThread, cdata);
    pthread_detach(acceptor);
    sleep(200);
#endif
    usbmuxd_device_info_t_dummy_ = (usbmuxd_device_info_t*)malloc(sizeof(usbmuxd_device_info_t));
    assert(usbmuxd_device_info_t_dummy_ != nullptr);
#if defined(WIN32) || defined(__CYGWIN__)
    SetGlobalLocalListenPort(local_port_);
#else
    SetGlobalLinuxListenPath(unix_socket_path_.c_str());
#endif
    if (!usbmuxd_get_device_by_udid(udid_.c_str(), (usbmuxd_device_info_t*)usbmuxd_device_info_t_dummy_))
      return false;
    usbmuxd_device_info_t* tmp = (usbmuxd_device_info_t*)usbmuxd_device_info_t_dummy_;
    SetProductId(tmp->product_id);
    SetHandle(tmp->handle);
    int sfd = usbmuxd_connect(tmp->handle, remote_port_);
    if (sfd == -1)
      return false;
    SetSockSFD(sfd);
    return true;
  }
  void UsbmuxdDevice::Release(){
    udid_.resize(0);
    local_port_ = 0;
    remote_port_ = 0;
    handle_ = 0;
    product_id_ = 0;
    if (sock_sfd_){
      usbmuxd_disconnect(sock_sfd_);
      sock_sfd_ = 0;
    }
    if (usbmuxd_device_info_t_dummy_){
      free(usbmuxd_device_info_t_dummy_);
      usbmuxd_device_info_t_dummy_ = nullptr;
    }
  }
  bool UsbmuxdDevice::IsOK(){
    return (udid_.size() && local_port_&&remote_port_&&handle_&&product_id_&&sock_sfd_&&usbmuxd_device_info_t_dummy_);
  }
  int UsbmuxdRW::Write(const char *data, std::uint32_t len, std::uint32_t *sent_bytes){
    return usbmuxd_send(sfd(), data, len, sent_bytes);
  }
  int UsbmuxdRW::ReadTimeout(char *data, uint32_t len, uint32_t *recv_bytes, unsigned int timeout){
    return usbmuxd_recv_timeout(sfd(), data, len, recv_bytes, timeout);
  }
  int UsbmuxdRW::Read(char *data, uint32_t len, uint32_t *recv_bytes){
    return usbmuxd_recv(sfd(), data, len, recv_bytes);
  }
  Multi_usbmuxd::Multi_usbmuxd(){
    connection_info_.resize(0);
  }

  Multi_usbmuxd::~Multi_usbmuxd(){
    std::vector<UsbmuxdDevice*>::iterator connection_info_it = connection_info_.begin();
    for (; connection_info_it != connection_info_.end(); connection_info_it++){
      (*connection_info_it)->Release();
      delete ((*connection_info_it));
    }
  }
  bool Multi_usbmuxd::AddDevice(const char* udid, const std::uint32_t local_port, const std::uint32_t remote_port){
    if (udid&&udid[0]){
      UsbmuxdDevice* connection_info = new UsbmuxdDevice;
      assert(connection_info != nullptr);
      connection_info->SetUdid(udid);
      connection_info->SetLocalPort(local_port);
      connection_info->SetRemotePort(remote_port);
      connection_info_.push_back(connection_info);
      return true;
    }
    return false;
  }
  bool Multi_usbmuxd::AddLinuxDevice(const char* udid, const char* socket_path, const std::uint32_t remote_port) {
    if (udid&&udid[0]&&socket_path&&socket_path[0]) {
      UsbmuxdDevice* connection_info = new UsbmuxdDevice;
      assert(connection_info != nullptr);
      connection_info->SetUdid(udid);
      connection_info->SetLinuxSocketPath(socket_path);
      connection_info->SetRemotePort(remote_port);
      connection_info_.push_back(connection_info);
      return true;
    }
    return false;
  }
  bool Multi_usbmuxd::ActivationDevice(const char* udid){
    bool result = false;
    const std::string udid_str = udid;
    std::vector<UsbmuxdDevice*>::iterator connection_info_it = connection_info_.begin();
    for (; connection_info_it != connection_info_.end(); connection_info_it++){
      if ((*connection_info_it)->udid() == udid_str){
        (*connection_info_it)->SetUsbmuxdDeviceinfo();
        result = (*connection_info_it)->IsOK();
        break;
      }
    }
    return result;
  }
  Multi_usbmuxd::ActivationStatusTable Multi_usbmuxd::ActivationStatus(const char* udid){
    const std::string udid_str = udid;
    std::vector<UsbmuxdDevice*>::iterator connection_info_it = connection_info_.begin();
    for (; connection_info_it != connection_info_.end(); connection_info_it++){
      if ((*connection_info_it)->udid() == udid_str){
        if ((*connection_info_it)->IsOK()){
          return Multi_usbmuxd::ActivationStatusTable::kOK;
        }
        break;
      }
    }
    return Multi_usbmuxd::ActivationStatusTable::kFail;
  }
  bool Multi_usbmuxd::DeactivationDevice(const char* udid){
    bool result = false;
    const std::string udid_str = udid;
    std::vector<UsbmuxdDevice*>::iterator connection_info_it = connection_info_.begin();
    for (; connection_info_it != connection_info_.end(); connection_info_it++){
      if ((*connection_info_it)->udid() == udid_str){
        (*connection_info_it)->Release();
        delete ((*connection_info_it));
        result = true;
        break;
      }
    }
    return result;
  }
  bool Multi_usbmuxd::GetUsbmuxdRW(const char* udid, UsbmuxdRW& usbmuxd_rw){
    const std::string udid_str = udid;
    std::vector<UsbmuxdDevice*>::iterator connection_info_it = connection_info_.begin();
    for (; connection_info_it != connection_info_.end(); connection_info_it++){
      if ((*connection_info_it)->udid() == udid_str){
        usbmuxd_rw.SetSFD((*connection_info_it)->sock_sfd());
        return true;
      }
    }
    return false;
  }
}