#include "napi/native_api.h"
#include "hilog/log.h"
#include <pthread.h>
#include <cstdlib>
#include <ctime>

pthread_mutex_t g_mutex_ = PTHREAD_MUTEX_INITIALIZER;

napi_ref g_cbObj = nullptr;
napi_threadsafe_function g_tsfn;

#define LOG_TAG "SpeedTestDemo"

struct delay_data {
    double delay;
    int min;
    int max;
    double jitter;
};

const int DELAY_ARGC = 4;

static void *speedtest_run(void *args);

static napi_value run_speedtest(napi_env env, napi_callback_info info) {
    if (g_cbObj == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, LOG_TAG, "cbObj == nullptr");
        return nullptr;
    }
    size_t argc = 0;
    napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);

    pthread_t tid;
    pthread_create(&tid, NULL, &speedtest_run, NULL);
    
    return nullptr;
}

static void set_delay_cb_call_js(napi_env env, napi_value js_cb, void *context, void *data) {
    napi_get_reference_value(env, g_cbObj, &js_cb);
    delay_data *delay_data_ = (delay_data *)data;
    // can also get result from 4th arg (void *data)

    napi_value res[DELAY_ARGC] = {0};
    
    napi_create_double(env, delay_data_->delay, &res[0]);
    napi_create_int32(env, delay_data_->min, &res[1]);
    napi_create_int32(env, delay_data_->max, &res[2]);
    napi_create_double(env, delay_data_->jitter, &res[3]);
    
    napi_call_function(env, nullptr, js_cb, DELAY_ARGC, res, nullptr);
    
    delete delay_data_; // avoid mem leak
}

static void *speedtest_run(void *args) {
    pthread_detach(pthread_self());
    static int i = 0;
    
    pthread_mutex_lock(&g_mutex_);
    
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, LOG_TAG, "SpeedTestRunner i = %{public}d", i);
    
    delay_data *data = new delay_data; // remember to delete it in set_delay_cb_call_js
    if (data == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, LOG_TAG, "SpeedTestRunner new delay_data failed");
        return nullptr;
    }
    
    std::srand(std::time(nullptr));
    double delay = std::rand() % 50 + std::rand() % 100 / 100.0 + 10;
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, LOG_TAG, "SpeedTestRunner delay = %{public}f", delay);
    
    data->delay = delay;
    data->min = (int)delay - 10;
    data->max = (int)delay + 10;
    data->jitter = std::rand() % 10 + std::rand() % 100 / 100.0;
    
    napi_acquire_threadsafe_function(g_tsfn);
    napi_call_threadsafe_function(g_tsfn, data, napi_tsfn_nonblocking);
    
    i++;
    
    pthread_mutex_unlock(&g_mutex_);

    return nullptr;
}

static napi_value register_set_delay_cb(napi_env env, napi_callback_info info) {
    size_t argc = 1; // count of cb
    napi_value js_cb, work_name;
    napi_get_cb_info(env, info, &argc, &js_cb, nullptr, nullptr);
    napi_create_reference(env, js_cb, 1, &g_cbObj);
    napi_create_string_utf8(env, "delay_cb", NAPI_AUTO_LENGTH, &work_name);
    napi_create_threadsafe_function(env, js_cb, NULL, work_name, 0, 1, NULL, NULL, NULL, set_delay_cb_call_js, &g_tsfn);
    return nullptr;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports) {
    napi_property_descriptor desc[] = {
        {"registerSetDelayCb", nullptr, register_set_delay_cb, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"runSpeedTest", nullptr, run_speedtest, nullptr, nullptr, nullptr, napi_default, nullptr}};
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module demoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "entry",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterEntryModule(void) { napi_module_register(&demoModule); }