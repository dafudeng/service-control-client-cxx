/* Copyright 2017 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef GOOGLE_SERVICE_CONTROL_CLIENT_SERVICE_CONTROL_CLIENT_IMPL_H_
#define GOOGLE_SERVICE_CONTROL_CLIENT_SERVICE_CONTROL_CLIENT_IMPL_H_

#include "include/service_control_client.h"
#include "src/quota_aggregator_impl.h"
#include "utils/google_macros.h"

#include <atomic>

namespace google {
namespace service_control_client {

// ServiceControlClient implementation class.
// Thread safe.
class ServiceControlClientImpl : public ServiceControlClient {
 public:
  // Constructor.
  ServiceControlClientImpl(const std::string& service_name,
                           const std::string& service_config_id,
                           ServiceControlClientOptions& options);

  // Override the destructor.
  virtual ~ServiceControlClientImpl();

  // An async check call.
  virtual void Check(
      const ::google::api::servicecontrol::v1::CheckRequest& check_request,
      ::google::api::servicecontrol::v1::CheckResponse* check_response,
      DoneCallback on_check_done);

  // A check call with per_request transport.
  virtual void Check(
      const ::google::api::servicecontrol::v1::CheckRequest& check_request,
      ::google::api::servicecontrol::v1::CheckResponse* check_response,
      DoneCallback on_check_done, TransportCheckFunc check_transport);

  // An async quota call.
  virtual void Quota(
      const ::google::api::servicecontrol::v1::AllocateQuotaRequest&
          quota_request,
      ::google::api::servicecontrol::v1::AllocateQuotaResponse* quota_response,
      DoneCallback on_quota_done);

  // A quota call with per_request transport.
  virtual void Quota(
      const ::google::api::servicecontrol::v1::AllocateQuotaRequest&
          quota_request,
      ::google::api::servicecontrol::v1::AllocateQuotaResponse* quota_response,
      DoneCallback on_quota_done, TransportQuotaFunc quota_transport);

  // An async report call.
  virtual void Report(
      const ::google::api::servicecontrol::v1::ReportRequest& report_request,
      ::google::api::servicecontrol::v1::ReportResponse* report_response,
      DoneCallback on_report_done);

  virtual ::google::protobuf::util::Status GetStatistics(
      Statistics* stat) const;

  // A report call with per_request transport.
  virtual void Report(
      const ::google::api::servicecontrol::v1::ReportRequest& report_request,
      ::google::api::servicecontrol::v1::ReportResponse* report_response,
      DoneCallback on_report_done, TransportReportFunc report_transport);

 private:
  ::google::protobuf::util::Status convertResponseStatus(
      const ::google::api::servicecontrol::v1::AllocateQuotaResponse& response);

  // A flush callback for check.
  void CheckFlushCallback(
      const ::google::api::servicecontrol::v1::CheckRequest& check_request);

  // A flush callback for check.
  void AllocateQuotaFlushCallback(
      const ::google::api::servicecontrol::v1::AllocateQuotaRequest&
          quota_request);

  // A flush callback for report.
  void ReportFlushCallback(
      const ::google::api::servicecontrol::v1::ReportRequest& report_request);

  // Gets next flush interval
  int GetNextFlushInterval();

  // Flushes out all items.
  google::protobuf::util::Status FlushAll();

  // Flushes out expired items.
  ::google::protobuf::util::Status Flush();

  std::string service_name_;

  // The check transport function.
  TransportQuotaFunc quota_transport_;
  // The check transport function.
  TransportCheckFunc check_transport_;
  // The report transport function.
  TransportReportFunc report_transport_;

  // The Timer object.
  std::shared_ptr<PeriodicTimer> flush_timer_;

  // Atomic object to deal with multi-threads situation.
  std::atomic_int_fast64_t total_called_quotas_;
  std::atomic_int_fast64_t send_quotas_by_flush_;
  std::atomic_int_fast64_t send_quotas_in_flight_;

  std::atomic_int_fast64_t total_called_checks_;
  std::atomic_int_fast64_t send_checks_by_flush_;
  std::atomic_int_fast64_t send_checks_in_flight_;

  std::atomic_int_fast64_t total_called_reports_;
  std::atomic_int_fast64_t send_reports_by_flush_;
  std::atomic_int_fast64_t send_reports_in_flight_;
  std::atomic_int_fast64_t send_report_operations_;

  // The check aggregator object. Uses shared_ptr for check_aggregator_.
  // Transport::on_check_done() callback needs to call check_aggregator_
  // CacheResponse() function. The callback function needs to hold a ref_count
  // of check_aggregator_ to make sure it is not freed.
  std::shared_ptr<CheckAggregator> check_aggregator_;

  std::shared_ptr<QuotaAggregator> quota_aggregator_;

  // The report aggregator object. report_aggregator_ has to be shared_ptr since
  // it will be passed to flush_timer callback.
  std::shared_ptr<ReportAggregator> report_aggregator_;

  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(ServiceControlClientImpl);
};

}  // namespace service_control_client
}  // namespace google

#endif  // GOOGLE_SERVICE_CONTROL_CLIENT_SERVICE_CONTROL_CLIENT_IMPL_H_
