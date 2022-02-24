#include <assert.h>
#include <ctype.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "io_transwarp_security_graphene_GrapheneJNI.h"

static bool getenv_client_inside_sgx() {
  char *str = getenv("RA_TLS_CLIENT_INSIDE_SGX");
  if (!str)
    return false;

  return !strcmp(str, "1") || !strcmp(str, "true") || !strcmp(str, "TRUE");
}

static int (*ra_tls_verify_callback_der_f)(uint8_t* der_crt, size_t der_crt_size);
static void (*ra_tls_set_measurement_callback_f)(int (*f_cb)(const char* mrenclave, const char* mrsigner,
                                                      const char* isv_prod_id, const char* isv_svn));

static char g_expected_mrenclave[32];
static char g_expected_mrsigner[32];
static char g_expected_isv_prod_id[2];
static char g_expected_isv_svn[2];

static int my_verify_measurements(const char* mrenclave, const char* mrsigner,
                                  const char* isv_prod_id, const char* isv_svn) {
  assert(mrenclave && mrsigner && isv_prod_id && isv_svn);

  if (memcmp(mrenclave, g_expected_mrenclave, sizeof(g_expected_mrenclave)) != 0)
    return -1;

  if (memcmp(mrsigner, g_expected_mrsigner, sizeof(g_expected_mrsigner)) != 0)
    return -1;

  if (memcmp(isv_prod_id, g_expected_isv_prod_id, sizeof(g_expected_isv_prod_id)) != 0)
    return -1;

  if (memcmp(isv_svn, g_expected_isv_svn, sizeof(g_expected_isv_svn)) != 0)
    return -1;

  return 0;
}

static int parse_hex(const char* hex, void* buffer, size_t buffer_size) {
  if (strlen(hex) != buffer_size * 2)
    return -1;

  for (size_t i = 0; i < buffer_size; i++) {
    if (!isxdigit(hex[i * 2]) || !isxdigit(hex[i * 2 + 1]))
      return -1;
    sscanf(hex + i * 2, "%02hhx", &((uint8_t*)buffer)[i]);
  }
  return 0;
}

JNIEXPORT jboolean JNICALL Java_io_transwarp_security_graphene_GrapheneJNI_initClient(JNIEnv *env, jobject object,
                                                                              jstring mrenclave, jstring mrsigner, jstring isv_prod_id, jstring isv_svn) {
  void *ra_tls_verify_lib;
  bool in_sgx = getenv_client_inside_sgx();
  if (in_sgx) {
    ra_tls_verify_lib = dlopen("libra_tls_verify_dcap_graphene.so", RTLD_LAZY);
    if (!ra_tls_verify_lib) {
      printf("%s\n", dlerror());
      printf("User requested RA-TLS verification with DCAP inside SGX but cannot find lib\n");
      printf("Please make sure that you are using client_dcap.manifest\n");
      return JNI_FALSE;
    }
  } else {
    void *helper_sgx_urts_lib = dlopen("libsgx_urts.so", RTLD_NOW | RTLD_GLOBAL);
    if (!helper_sgx_urts_lib) {
      printf("%s\n", dlerror());
      printf("User requested RA-TLS verification with DCAP but cannot find helper"
                     " libsgx_urts.so lib\n");
      return JNI_FALSE;
    }

    ra_tls_verify_lib = dlopen("libra_tls_verify_dcap.so", RTLD_LAZY);
    if (!ra_tls_verify_lib) {
      printf("%s\n", dlerror());
      printf("User requested RA-TLS verification with DCAP but cannot find lib\n");
      return JNI_FALSE;
    }
  }

  if (ra_tls_verify_lib) {
    char *error;
    ra_tls_verify_callback_der_f = dlsym(ra_tls_verify_lib, "ra_tls_verify_callback_der");
    if ((error = dlerror()) != NULL) {
      printf("%s\n", error);
      return JNI_FALSE;
    }

    ra_tls_set_measurement_callback_f = dlsym(ra_tls_verify_lib, "ra_tls_set_measurement_callback");
    if ((error = dlerror()) != NULL) {
      printf("%s\n", error);
      return JNI_FALSE;
    }

    if (mrenclave != NULL) {
      const char *str = (*env)->GetStringUTFChars(env, mrenclave, JNI_FALSE);
      if (parse_hex(str, g_expected_mrenclave, sizeof(g_expected_mrenclave)) < 0) {
        printf("Cannot parse MRENCLAVE!\n");
        return JNI_FALSE;
      }
      (*env)->ReleaseStringUTFChars(env, mrenclave, str);
    }
    if (mrsigner != NULL) {
      const char *str = (*env)->GetStringUTFChars(env, mrsigner, JNI_FALSE);
      if (parse_hex(str, g_expected_mrsigner, sizeof(g_expected_mrsigner)) < 0) {
        printf("Cannot parse MRSIGNER!\n");
        return JNI_FALSE;
      }
      (*env)->ReleaseStringUTFChars(env, mrsigner, str);
    }
    if (isv_prod_id != NULL) {
      const char *str = (*env)->GetStringUTFChars(env, isv_prod_id, JNI_FALSE);
      errno = 0;
      uint16_t d = (uint16_t)strtoul(str, NULL, 10);
      if (errno) {
        printf("Cannot parse ISV_PROD_ID!\n");
        return 1;
      }
      memcpy(g_expected_isv_prod_id, &d, sizeof(d));
      (*env)->ReleaseStringUTFChars(env, isv_prod_id, str);
    }
    if (isv_svn != NULL) {
      const char *str = (*env)->GetStringUTFChars(env, isv_svn, JNI_FALSE);
      errno = 0;
      uint16_t d = (uint16_t)strtoul(str, NULL, 10);
      if (errno) {
        printf("Cannot parse ISV_SVN\n");
        return 1;
      }
      memcpy(g_expected_isv_svn, &d, sizeof(d));
      (*env)->ReleaseStringUTFChars(env, isv_svn, str);
    }

    if (in_sgx) {
      ra_tls_set_measurement_callback_f(NULL);
    } else {
      ra_tls_set_measurement_callback_f(my_verify_measurements);
    }
  }
  return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_io_transwarp_security_graphene_GrapheneJNI_initServer(JNIEnv *env, jobject object, jobject key_buffer, jobject crt_buffer) {
  void *ra_tls_attest_lib = dlopen("libra_tls_attest.so", RTLD_LAZY);
  if (!ra_tls_attest_lib) {
    printf("User requested RA-TLS attestation but cannot find lib\n");
    return JNI_FALSE;
  }

  char* error;
  int (*ra_tls_create_key_and_crt_der_f)(uint8_t** der_key, size_t* der_key_size, uint8_t** der_crt,
                                    size_t* der_crt_size);
  ra_tls_create_key_and_crt_der_f = dlsym(ra_tls_attest_lib, "ra_tls_create_key_and_crt_der");
  if ((error = dlerror()) != NULL) {
    printf("%s\n", error);
    return JNI_FALSE;
  }

  uint8_t *der_key, *der_crt;
  size_t der_key_size, der_crt_size;
  int ret = ra_tls_create_key_and_crt_der_f(&der_key, &der_key_size, &der_crt, &der_crt_size);
  if (ret != 0) {
    printf(" failed\n  !  ra_tls_create_key_and_crt_der returned %d\n\n", ret);
    return JNI_FALSE;
  }

  jlong key_len = (*env)->GetDirectBufferCapacity(env, key_buffer);
  jlong crt_len = (*env)->GetDirectBufferCapacity(env, crt_buffer);
  if (key_len < der_key_size || crt_len < der_crt_size) {
    free(der_key);
    free(der_crt);
    return JNI_FALSE;
  }

  jclass bbclass = (*env)->FindClass(env, "java/nio/ByteBuffer");
  jmethodID putMethod = (*env)->GetMethodID(env, bbclass, "put", "([BII)Ljava/nio/ByteBuffer;");

  jbyteArray data = (*env)->NewByteArray(env, der_key_size);
  (*env)->SetByteArrayRegion(env, data, 0, der_key_size, der_key);
  (*env)->CallObjectMethod(env, key_buffer, putMethod, data, 0, der_key_size);
  (*env)->DeleteLocalRef(env, data);

  data = (*env)->NewByteArray(env, der_crt_size);
  (*env)->SetByteArrayRegion(env, data, 0, der_crt_size, der_crt);
  (*env)->CallObjectMethod(env, crt_buffer, putMethod, data, 0, der_crt_size);
  (*env)->DeleteLocalRef(env, data);

  free(der_key);
  free(der_crt);

  return JNI_TRUE;
}

JNIEXPORT jint JNICALL Java_io_transwarp_security_graphene_GrapheneJNI_raTlsVerify(JNIEnv *env, jobject object, jbyteArray der_crt) {
  jbyte *jb = (*env)->GetByteArrayElements(env, der_crt, JNI_FALSE);
  int jb_len = (*env)->GetArrayLength(env, der_crt);
//  char data[jb_len + 1];
//  memcpy(data, jb, jb_len + 1);
//  data[jb_len] = 0;
  int ret = ra_tls_verify_callback_der_f(jb, jb_len);
  (*env)->ReleaseByteArrayElements(env, der_crt, jb, 0);

  //return ra_tls_verify_callback_der_f(data, jb_len);
  return ret;
}