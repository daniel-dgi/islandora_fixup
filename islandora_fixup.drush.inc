<?php
/**
 * @file
 * Drupal batch to fixup some files.
 */

/**
 * Implements hook_drush_command().
 */
function islandora_fixup_drush_command() {
  $items = array();
  $items['islandora_fixup_batch'] = array(
    'description' => 'Fixes up items based on a pid list in SQL.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );
  return $items;
}

/**
 * Drush command callback.
 *
 * Sets up the batch to touch up some files.
 */
function drush_islandora_fixup_batch()
{
  // Set batch.
  batch_set(array(
    'finished' => 'islandora_fixup_batch_finished',
    'file' => drupal_get_path('module', 'islandora_fixup') . 'islandora_fixup.drush.inc',
    'operations' => array(
      array(
        'islandora_fixup_batch_operation',
        array(),
      ),
    ),
  ));
  $batch =& batch_get();
  $batch['progressive'] = FALSE;
  drush_backend_batch_process();
}

/**
 * Batch operation for islandora_fixup.
 *
 * Queries the db for a handful of files and process them.
 *
 * @param array $context
 *   Batch context.  Added automatically by the batch api.
 */
function islandora_fixup_batch_operation(array &$context) {
  // Init progress if required.
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $total = db_select('islandora_fixup_list', 'ifl')
      ->condition('fixed', 0)
      ->countQuery()
      ->execute()
      ->fetchField();
    $context['sandbox']['total'] = $total;
    drush_print("STARTING AT 0, TOTAL $total");
  }

  drush_print("STARTING A NEW BATCH OPERATION");

  $limit = 10;
  $offset = $context['sandbox']['progress'];

  drush_print("LIMIT: $limit");
  drush_print("OFFSET: $offset");
 
  $query = "SELECT * FROM islandora_fixup_list WHERE fixed = 0";
  $rows = db_query_range($query, $offset, $limit)->fetchAll();

  foreach ($rows as $row) {
    try {

    } catch (Exception $e) {
      db_update('islandora_fixup_list')
        ->fields(array(
          'fixed' => 2,
        ))
        ->condition('pid', $pid)
        ->execute();
    }
    $pid = $row->pid;
    drush_print("WORKING ON $pid");

    $obj = islandora_object_load($pid);
    
    // do something here
    $obj->label = $obj->label . "!";
 
    db_update('islandora_fixup_list')
      ->fields(array(
        'fixed' => 1,
      ))
      ->condition('pid', $pid)
      ->execute();
  }

  // Update progress.
  if ($context['sandbox']['total'] == 0) {
    $context['finished'] = 1;
  }
  else {
    $context['sandbox']['progress'] += $limit;
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['total'];
  }

}

/**
 * Batch finished function for fixup.
 */
function islandora_fixup_batch_finished($success, $results, $operations) {
  drush_print("FINISHED");
}

