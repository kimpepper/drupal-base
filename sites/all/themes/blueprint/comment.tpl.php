<div class="clear-block comment<?php print ($comment->new) ? ' comment-new' : ''; print(isset($comment->status) && $comment->status == COMMENT_NOT_PUBLISHED) ? ' comment-unpublished' : ''; if (isset($author_comment)) print ' author'; print ' '. $zebra; ?>">
  <div class="picture span-3">
    <?php print $picture ?>
  </div>

  <div class="comment-content">
    <div class="meta">
      <div class="permalink clearfix">
        <?php if ($comment->new) : ?>
          <a id="new"></a>
          <span class="new"><?php print drupal_ucfirst($new) ?></span>
        <?php endif; ?>
        <?php print l('#'. $comment_count, 'node/'. $comment->nid, array('fragment' => 'comment-'. $comment->cid)); ?>
      </div>
      <?php if ($submitted): ?>
        <span class="username"><?php print theme('username', $comment); ?></span> <span class="date"><?php print t('wrote !date ago', array( '!date' => format_interval(time() - $comment->timestamp))); ?></span>
      <?php endif; ?>
    </div>
    
    <div class="content">
      <?php if ($title): ?><h3><?php print $title; ?></h3><?php endif; ?>
      <?php print $content ?>
      <?php if ($signature): ?>
        <div class="user-signature clear-block">
          <?php print $signature ?>
        </div>
      <?php endif; ?>
    </div>

    <?php if ($links): ?>
      <div class="links"><?php print $links ?></div>
    <?php endif; ?>
  </div>
</div>
