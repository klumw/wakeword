/* The Clear BSD License
 *
 * Copyright (c) 2025 EdgeImpulse Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 *   * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#define FLATBUFFERS_LOCALE_INDEPENDENT 0
#include "edge-impulse-sdk/third_party/flatbuffers/include/flatbuffers/flexbuffers.h"
#include "edge-impulse-sdk/tensorflow/lite/c/common.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/kernel_util.h"
#include <math.h>

#define FEATURE_TYPE float

namespace tflite {
namespace {

struct OpDataTree {
  uint32_t num_leaf_nodes;
  uint32_t num_internal_nodes;
  uint32_t num_trees;
  const uint16_t* nodes_modes;
  const uint16_t* nodes_featureids;
  const float* nodes_values;
  const uint16_t* nodes_truenodeids;
  const uint16_t* nodes_falsenodeids;
  const float* nodes_weights;
  const uint8_t* nodes_classids;
  const uint16_t* tree_root_ids;
  const uint8_t* buffer_t;
  size_t buffer_length;
};

void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  const uint8_t* buffer_t = reinterpret_cast<const uint8_t*>(buffer);
  const flexbuffers::Map& m = flexbuffers::GetRoot(buffer_t, length).AsMap();

  TFLITE_DCHECK(context->AllocatePersistentBuffer != nullptr);
  OpDataTree* data = static_cast<OpDataTree*>(context->AllocatePersistentBuffer(context, sizeof(OpDataTree)));

  data->buffer_t = buffer_t;
  data->buffer_length = length;

  data->num_leaf_nodes = m["num_leaf_nodes"].AsUInt32();
  data->num_internal_nodes = m["num_internal_nodes"].AsUInt32();
  data->num_trees = m["num_trees"].AsUInt32();

  data->nodes_modes = (uint16_t*)(m["nodes_modes"].AsBlob().data());
  data->nodes_featureids = (uint16_t*)(m["nodes_featureids"].AsBlob().data());
  data->nodes_values = (float*)(m["nodes_values"].AsBlob().data());
  data->nodes_truenodeids = (uint16_t*)(m["nodes_truenodeids"].AsBlob().data());
  data->nodes_falsenodeids = (uint16_t*)(m["nodes_falsenodeids"].AsBlob().data());
  data->nodes_weights = (float*)(m["nodes_weights"].AsBlob().data());
  data->nodes_classids = (uint8_t*)(m["nodes_classids"].AsBlob().data());
  data->tree_root_ids = (uint16_t*)(m["tree_root_ids"].AsBlob().data());

  return data;
}

TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {

  MicroContext* micro_context = GetMicroContext(context);
  const OpDataTree* data = static_cast<const OpDataTree*>(node->user_data);
  const flexbuffers::Map& m = flexbuffers::GetRoot(data->buffer_t, data->buffer_length).AsMap();

  // The OOB checks below are very important to prevent vulnerabilities where an adversary sends
  // us a malicious TFLite model, similar to: https://nvd.nist.gov/vuln/detail/CVE-2022-23560

  int num_nodes = data->num_leaf_nodes + data->num_internal_nodes;

  // Check that the tree root ids are valid.
  for (uint32_t i = 0; i < data->num_trees; i++) {
    TF_LITE_ENSURE_EQ(context, data->tree_root_ids[i] < num_nodes, true);
    TF_LITE_ENSURE_EQ(context, data->tree_root_ids[i] >= 0, true);
  }

  // Check that all node indices are valid
  for (uint32_t i = 0; i < data->num_internal_nodes; i++) {
    TF_LITE_ENSURE_EQ(context, data->nodes_truenodeids[i] < num_nodes, true);
    TF_LITE_ENSURE_EQ(context, data->nodes_truenodeids[i] >= 0, true);
    TF_LITE_ENSURE_EQ(context, data->nodes_falsenodeids[i] < num_nodes, true);
    TF_LITE_ENSURE_EQ(context, data->nodes_falsenodeids[i] >= 0, true);
  }

  // Check all node arrays have the same length
  TF_LITE_ENSURE_EQ(context, data->num_internal_nodes, m["nodes_featureids"].AsBlob().size());
  TF_LITE_ENSURE_EQ(context, data->num_internal_nodes, m["nodes_values"].AsBlob().size());
  TF_LITE_ENSURE_EQ(context, data->num_internal_nodes, m["nodes_truenodeids"].AsBlob().size());
  TF_LITE_ENSURE_EQ(context, data->num_internal_nodes, m["nodes_falsenodeids"].AsBlob().size());
  TF_LITE_ENSURE_EQ(context, data->num_leaf_nodes, m["nodes_weights"].AsBlob().size());
  TF_LITE_ENSURE_EQ(context, data->num_leaf_nodes, m["nodes_classids"].AsBlob().size());

  // Check data types are supported. Currently we only support one combination.
  TF_LITE_ENSURE_EQ(context, strncmp(m["tree_index_type"].AsString().c_str(), "uint16", 6), 0);
  TF_LITE_ENSURE_EQ(context, strncmp(m["node_value_type"].AsString().c_str(), "float32", 7), 0);
  TF_LITE_ENSURE_EQ(context, strncmp(m["class_index_type"].AsString().c_str(), "uint8", 5), 0);
  TF_LITE_ENSURE_EQ(context, strncmp(m["class_weight_type"].AsString().c_str(), "float32", 7), 0);
  TF_LITE_ENSURE_EQ(context, strncmp(m["equality_operator"].AsString().c_str(), "leq", 3), 0);

  TF_LITE_ENSURE_EQ(context, NumInputs(node), 1);
  TF_LITE_ENSURE_EQ(context, NumOutputs(node), 1);
  TfLiteTensor* input = micro_context->AllocateTempInputTensor(node, 0);
  TF_LITE_ENSURE(context, input != nullptr);
  TF_LITE_ENSURE(context, NumDimensions(input) == 2);
  TfLiteTensor* output = micro_context->AllocateTempOutputTensor(node, 0);
  TF_LITE_ENSURE(context, output != nullptr);

  int input_width = SizeOfDimension(input, 1);
  int output_width = SizeOfDimension(output, 1);

  // Check that all indices into the input/output tensor are valid
  for (uint32_t i = 0; i < data->num_internal_nodes; i++) {
    TF_LITE_ENSURE(context, data->nodes_featureids[i] < input_width);
    TF_LITE_ENSURE(context, data->nodes_featureids[i] >= 0);
    if (!m["nodes_modes"].AsBlob().IsTheEmptyBlob()) {
        if (data->nodes_modes[i] == 0) {
            TF_LITE_ENSURE(context, data->nodes_classids[i] < output_width);
            TF_LITE_ENSURE(context, data->nodes_classids[i] >= 0);
        }
    }
  }

  micro_context->DeallocateTempTfLiteTensor(input);
  micro_context->DeallocateTempTfLiteTensor(output);

  return kTfLiteOk;
}

TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {

  const OpDataTree* data = static_cast<const OpDataTree*>(node->user_data);

  const TfLiteEvalTensor* input =
      tflite::micro::GetEvalInput(context, node, 0);
  const float *in_data = tflite::micro::GetTensorData<float>(input);

  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, 0);
  float *out_data = tflite::micro::GetTensorData<float>(output);

  const tflite::RuntimeShape output_shape = tflite::micro::GetTensorShape(output);
  memset(out_data, 0, output_shape.FlatSize() * sizeof(float));

  for (uint32_t i = 0; i < data->num_trees; i++) {
    uint16_t ix = data->tree_root_ids[i];

    while (ix < data->num_internal_nodes) {
      float node_val = 0;
      memcpy(&node_val, (data->nodes_values + ix), sizeof(float));

      if (in_data[data->nodes_featureids[ix]] <= node_val) {
        ix = data->nodes_truenodeids[ix];
      } else {
        ix = data->nodes_falsenodeids[ix];
      }
    }
    ix -= data->num_internal_nodes;

    float weight = 0;
    memcpy(&weight, (data->nodes_weights + ix), sizeof(float));
    out_data[data->nodes_classids[ix]] += weight;
  }

  return kTfLiteOk;
}


}  // namespace

TfLiteRegistration* Register_TreeEnsembleClassifier() {
  static TfLiteRegistration r = {Init,
          nullptr,
          Prepare,
          Eval,
          /*profiling_string=*/nullptr,
          /*builtin_code=*/0,
          /*custom_name=*/nullptr,
          /*version=*/0};
  return &r;
}

const char* GetString_TreeEnsembleClassifier() { return "TreeEnsembleClassifier"; }

}  // namespace tflite
