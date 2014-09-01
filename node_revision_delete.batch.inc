<?php

/**
 * @file
 * Batch API callbacks for node_revision_delete module.
 */

/**
 * Callback to delete revisions using Batch API.
 */
function node_revision_delete_batch_process($content_type, $max_revisions, &$context) {
  if (!isset($context['sandbox']['nids'])) {
    // Set initial values.
    $context['sandbox']['nids'] = node_revision_delete_candidates($content_type, $max_revisions);
    $context['sandbox']['current'] = 0;
    $context['sandbox']['total'] = count($context['sandbox']['nids']);
  }

  $nid = $context['sandbox']['nids'][$context['sandbox']['current']];
  $deleted_revisions = _node_revision_delete_do_delete($nid, $max_revisions);
  $context['sandbox']['deleted'][] = $deleted_revisions->count;
  if (empty($deleted_revisions->pending)) {
    $context['sandbox']['current']++;
  }

  // Inform the batch engine that we are not finished,
  // and provide an estimation of the completion level we reached.
  $context['finished'] = empty($context['sandbox']['nids']) ? 1 : $context['sandbox']['current'] / $context['sandbox']['total'];
}

/**
 * Batch 'finished' callback.
 */
function node_revision_delete_batch_finish($success, $results, $operations) {
  if ($success) {
    $message = count($results) . ' processed.';
    drupal_set_message($message);
  }
  else {
    // An error occurred.
    // $operations contains the operations that remained unprocessed.
    $error_operation = reset($operations);
    $message_variables = array(
      '%error_operation' => $error_operation[0],
      '@arguments' => print_r($error_operation[1], TRUE),
    );
    $message = t('An error occurred while processing %error_operation with arguments: @arguments', $message_variables, TRUE);
    drupal_set_message($message, 'error');
  }
}
