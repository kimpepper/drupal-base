<?php
// $Id: imagefield_file.inc,v 1.12 2009/03/15 06:50:37 quicksketch Exp $

/**
 * @file
 * hook_file and imagefield file functions.
 */

/**
 * Implementation of hook_file_references().
 */
function imagefield_file_references($file) {
  $references = 0;
  foreach(content_fields() as $field) {
    if ($field['type'] != 'imagefield') {
      continue;
    }
    $references += field_file_references($file, $field);
  }
  return array('imagefield' => $references);
}

/**
 * Implementation of hook_file_insert().
 */
function imagefield_file_insert($file) {
  // Currently empty. Thumbnails are now generated on preview.
}

/**
 * Implementation of hook_file_delete().
 *
 * Delete the admin thumbnail when the original is deleted.
 */
function imagefield_file_delete($file) {
  if (imagefield_file_is_image($file)) {
    file_delete(imagefield_file_admin_thumb_path($file, FALSE));
  }
}

/**
 * Simple utility function to check if a file is an image.
 */
function imagefield_file_is_image($file) {
  $file = (object)$file;
  return in_array($file->filemime, array('image/jpg', 'image/pjpeg', 'image/jpeg', 'image/png', 'image/gif'));
}

/**
 * Given a file, return the path the image thumbnail used while editing.
 */
function imagefield_file_admin_thumb_path($file, $create_thumb = TRUE) {
  $file = (object)$file;
  $short_path = preg_replace('/^' . preg_quote(file_directory_path(), '/') . '/', '', $file->filepath);
  $filepath = file_directory_path() . '/imagefield_thumbs' . $short_path;

  if ($create_thumb) {
    imagefield_create_admin_thumb($file->filepath, $filepath);
  }

  return $filepath;
}

/**
 * Create a thumbnail to be shown while editing an image.
 */
function imagefield_create_admin_thumb($source, $destination) {
  if (!is_file($source)) {
    return FALSE;
  }

  $info = image_get_info($source);
  $size = explode('x', variable_get('imagefield_thumb_size', '100x100'));

  // Check if the destination image needs to be regenerated to match a new size.
  if (is_file($destination)) {
    $thumb_info = image_get_info($destination);
    if ($thumb_info['width'] != $size[0] && $thumb_info['height'] != $size[1] && ($info['width'] > $size[0] || $info['height'] > $size[1])) {
      unlink($destination);
    }
    else {
      return;
    }
  }

  // Ensure the destination directory exists and is writable.
  $directories = explode('/', $destination);
  array_pop($directories); // Remove the file itself.
  foreach ($directories as $directory) {
    $full_path = isset($full_path) ? $full_path . '/' . $directory : $directory;
    file_check_directory($full_path, FILE_CREATE_DIRECTORY);
  }

  // Create the thumbnail.
  if ($info['width'] <= $size[0] || $info['height'] <= $size[1]) {
    file_copy($source, $destination);
  }
  else {
    image_scale($source, $destination, $size[0], $size[1]);
    // Set permissions. This is done for us when using file_copy().
    @chmod($destination, 0664);
  }
}
