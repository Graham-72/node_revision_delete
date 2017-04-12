<?php

namespace Drupal\node_revision_delete\Form;

use Drupal\Core\Form\ConfigFormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Render\RendererInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;
use \Drupal\Core\Url;

/**
 * Class NodeRevisionDeleteAdminSettings.
 *
 * @package Drupal\node_revision_delete\Form
 */
class NodeRevisionDeleteAdminSettings extends ConfigFormBase {

  /**
   * Drupal\Core\Render\RendererInterface definition.
   *
   * @var Drupal\Core\Render\RendererInterface
   */
  protected $renderer;

  /**
   * {@inheritdoc}
   */
  public function __construct(RendererInterface $renderer) {
    $this->renderer = $renderer;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('renderer')
    );
  }

  /**
   * {@inheritdoc}
   */
  protected function getEditableConfigNames() {
    return [
      'node_revision_delete.settings',
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'node_revision_delete_admin_settings';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    // Getting the config variables.
    $config = $this->config('node_revision_delete.settings');
    // Table header.
    $header = [
      $this->t('Content type'),
      $this->t('Machine name'),
      $this->t('Revisions to keep'),
      $this->t('Candidate nodes'),
      $this->t('Operations')
    ];
    // Table rows.
    $rows = [];
    // Searching the list with the maximum number of revisions to keep for each
    // content_type.
    $node_revision_delete_track = $config->get('node_revision_delete_track');

    // Looking for all the content types.
    $content_types = \Drupal::service('entity.manager')->getStorage('node_type')->loadMultiple();
    foreach ($content_types as $content_type) {
      $route_parameters = ['node_type' => $content_type->id()];
      // Return to the same page after save the content type.
      $destination = Url::fromRoute('node_revision_delete.admin_settings')->toString();
      $options = [
        'query' => ['destination' => $destination],
        'fragment' => 'edit-workflow',
      ];
      // Operations dropdown.
      $dropdown = [
        '#type' => 'dropbutton',
        '#links' => [
          // Action to edit the content type.
          'edit' => [
            'title' => $this->t('Edit'),
            'url' => Url::fromRoute('entity.node_type.edit_form', $route_parameters, $options),
          ],
          // Action to delete the configuration for the content type.
          'delete' => [
            'title' => $this->t('Untrack'),
            'url' => Url::fromUri('internal:/node/' . 1 . '/delete'),
          ],
        ],
      ];
      // Rendering the dropdown.
      $dropdown = $this->renderer->render($dropdown);
      // Searching the revisions to keep for each content type.
      $revisions_to_keep = isset($node_revision_delete_track[$content_type->id()]) ? $node_revision_delete_track[$content_type->id()] : $this->t('Untracked');
      // Setting the row values.
      $rows[] = [
        $content_type->label(),
        $content_type->id(),
        $revisions_to_keep,
        4,
        $dropdown,
      ];
    }
    // Table with current configuration.
    $form['current_configuration'] = [
      '#type' => 'table',
      '#caption' => $this->t('Current configuration'),
      '#header' => $header,
      '#rows' => $rows,
    ];
    // Available options for node_revision_delete_cron variable.
    $options_node_revision_delete_cron = [
      10 => 10,
      20 => 20,
      50 => 50,
      100 => 100,
      200 => 200,
      500 => 500,
      1000 => 1000,
    ];
    $form['node_revision_delete_cron'] = [
      '#type' => 'select',
      '#title' => $this->t('How many revisions do you want to delete per cron run?'),
      '#description' => $this->t('Deleting node revisions is a database intensive task. Increase this value if you think that the server can handle more deletions per cron run.'),
      '#options' => $options_node_revision_delete_cron,
      '#size' => 1,
      '#default_value' => $config->get('node_revision_delete_cron'),
    ];
    // Available options for node_revision_delete_time variable.
    $options_node_revision_delete_time = [
      'never' => $this->t('Never'),
      'every_time' => $this->t('Every time cron runs'),
      'everyday' => $this->t('Everyday'),
      'every_week' => $this->t('Every Week'),
      'every_10_days' => $this->t('Every 10 Days'),
      'every_15_days' => $this->t('Every 15 Days'),
      'every_month' => $this->t('Every Month'),
      'every_3_months' => $this->t('Every 3 Months'),
      'every_6_months' => $this->t('Every 6 Months'),
      'every_year' => $this->t('Every Year'),
    ];
    $form['node_revision_delete_time'] = [
      '#type' => 'select',
      '#title' => $this->t('How often should revision be deleted while cron runs?'),
      '#options' => $options_node_revision_delete_time,
      '#size' => 1,
      '#default_value' => $config->get('node_revision_delete_time'),
    ];
    // Providing the option to run now the batch job.
    $form['run_now'] = [
      '#type' => 'checkbox',
      '#title' => $this->t('Delete revisions now.'),
      '#description' => $this->t('This will start a batch job to delete old revisions for tracked content types.'),
    ];

    return parent::buildForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function validateForm(array &$form, FormStateInterface $form_state) {
    parent::validateForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    parent::submitForm($form, $form_state);
    // Saving the configuration.
    $this->config('node_revision_delete.settings')
      ->set('node_revision_delete_cron', $form_state->getValue('node_revision_delete_cron'))
      ->set('node_revision_delete_time', $form_state->getValue('node_revision_delete_time'))
      ->save();
  }

}
