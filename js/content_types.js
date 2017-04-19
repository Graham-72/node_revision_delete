/**
 * @file
 * Javascript for the node content editing form.
 */

(function ($, Drupal) {

  'use strict';

  /**
   * Behaviors for setting summaries on content type form.
   *
   * @type {Drupal~behavior}
   *
   * @prop {Drupal~behaviorAttach} attach
   *   Attaches summary behaviors on content type edit forms.
   */
//  Drupal.behaviors.contentTypes = {
//    attach: function (context) {
//      var $context = $(context);
//      // Provide the vertical tab summaries.
//      $context.find('#edit-workflow').drupalSetSummary(function (context) {
//        var vals = [];
//        $(context).find('#edit-node-revision-delete-track:checked').parent().each(function () {
//          vals.push(Drupal.checkPlain($(this).text().trim()));
//        });        
//        return vals.join(', ');
//      });
//    }
//  };

})(jQuery, Drupal);
