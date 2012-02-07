static int file_delta_new__from_one(
	git_diff_list *diff,
	git_status_t status,
	unsigned int attr,
	const git_oid *oid,
	const char *path)
	git_diff_delta *delta = git__calloc(1, sizeof(git_diff_delta));
	/* This fn is just for single-sided diffs */
	assert(status == GIT_STATUS_ADDED || status == GIT_STATUS_DELETED);
	if (!delta)
		return git__rethrow(GIT_ENOMEM, "Could not allocate diff record");
	if ((delta->path = git__strdup(path)) == NULL) {
		git__free(delta);
		return git__rethrow(GIT_ENOMEM, "Could not allocate diff record path");
	}

	if (diff->opts.flags & GIT_DIFF_REVERSE)
		status = (status == GIT_STATUS_ADDED) ?
			GIT_STATUS_DELETED : GIT_STATUS_ADDED;
	delta->status = status;

	if (status == GIT_STATUS_ADDED) {
		delta->new_attr = attr;
		git_oid_cpy(&delta->new_oid, oid);
	} else {
		delta->old_attr = attr;
		git_oid_cpy(&delta->old_oid, oid);
	}

	if ((error = git_vector_insert(&diff->files, delta)) < GIT_SUCCESS)
	return error;
}

static int file_delta_new__from_tree_diff(
	git_diff_list *diff,
	const git_tree_diff_data *tdiff)
{
	int error;
	git_diff_delta *delta = git__calloc(1, sizeof(git_diff_delta));

	if (!delta)
		return git__rethrow(GIT_ENOMEM, "Could not allocate diff record");

	if ((diff->opts.flags & GIT_DIFF_REVERSE) == 0) {
		delta->status   = tdiff->status;
		delta->old_attr = tdiff->old_attr;
		delta->new_attr = tdiff->new_attr;
		delta->old_oid  = tdiff->old_oid;
		delta->new_oid  = tdiff->new_oid;
	} else {
		/* reverse the polarity of the neutron flow */
		switch (tdiff->status) {
		case GIT_STATUS_ADDED:   delta->status = GIT_STATUS_DELETED; break;
		case GIT_STATUS_DELETED: delta->status = GIT_STATUS_ADDED; break;
		default:                 delta->status = tdiff->status;
		}
		delta->old_attr = tdiff->new_attr;
		delta->new_attr = tdiff->old_attr;
		delta->old_oid  = tdiff->new_oid;
		delta->new_oid  = tdiff->old_oid;
	}

	delta->path = git__strdup(diff->pfx.ptr);
	if (delta->path == NULL) {
		git__free(delta);
		return git__rethrow(GIT_ENOMEM, "Could not allocate diff record path");
	}

	if ((error = git_vector_insert(&diff->files, delta)) < GIT_SUCCESS)
		file_delta_free(delta);
static int tree_walk_cb(const char *root, git_tree_entry *entry, void *data)
	if (S_ISDIR(git_tree_entry_attributes(entry)))
	/* join pfx, root, and entry->filename into one */
		(error = git_buf_joinpath(
			&diff->pfx, diff->pfx.ptr, git_tree_entry_name(entry))))
	error = file_delta_new__from_one(
		diff, diff->mode, git_tree_entry_attributes(entry),
		git_tree_entry_id(entry), diff->pfx.ptr);
static int tree_diff_cb(const git_tree_diff_data *tdiff, void *data)
	error = git_buf_joinpath(&diff->pfx, diff->pfx.ptr, tdiff->path);
	if (S_ISDIR(tdiff->old_attr) && S_ISDIR(tdiff->new_attr)) {
		if (!(error = git_tree_lookup(&old, diff->repo, &tdiff->old_oid)) &&
			!(error = git_tree_lookup(&new, diff->repo, &tdiff->new_oid)))
	} else if (S_ISDIR(tdiff->old_attr) || S_ISDIR(tdiff->new_attr)) {
		git_tree *tree     = NULL;
		int added_dir      = S_ISDIR(tdiff->new_attr);
		const git_oid *oid = added_dir ? &tdiff->new_oid : &tdiff->old_oid;
		diff->mode         = added_dir ? GIT_STATUS_ADDED : GIT_STATUS_DELETED;

		if (!(error = git_tree_lookup(&tree, diff->repo, oid)))
			error = git_tree_walk(tree, tree_walk_cb, GIT_TREEWALK_POST, diff);
		git_tree_free(tree);
	} else
		error = file_delta_new__from_tree_diff(diff, tdiff);
static char *git_diff_src_prefix_default = "a/";
static char *git_diff_dst_prefix_default = "b/";
#define PREFIX_IS_DEFAULT(A) \
	((A) == git_diff_src_prefix_default || (A) == git_diff_dst_prefix_default)

static char *copy_prefix(const char *prefix)
{
	size_t len = strlen(prefix);
	char *str = git__malloc(len + 2);
	if (str != NULL) {
		memcpy(str, prefix, len + 1);
		/* append '/' at end if needed */
		if (len > 0 && str[len - 1] != '/') {
			str[len] = '/';
			str[len + 1] = '\0';
		}
	}
	return str;
}

	if (diff == NULL)
		return NULL;

	diff->repo = repo;
	git_buf_init(&diff->pfx, 0);

	if (opts == NULL)
		return diff;

	memcpy(&diff->opts, opts, sizeof(git_diff_options));

	diff->opts.src_prefix = (opts->src_prefix == NULL) ?
		git_diff_src_prefix_default : copy_prefix(opts->src_prefix);
	diff->opts.dst_prefix = (opts->dst_prefix == NULL) ?
		git_diff_dst_prefix_default : copy_prefix(opts->dst_prefix);
	if (!diff->opts.src_prefix || !diff->opts.dst_prefix) {
		git__free(diff);
		return NULL;
	}
	if (diff->opts.flags & GIT_DIFF_REVERSE) {
		char *swap = diff->opts.src_prefix;
		diff->opts.src_prefix = diff->opts.dst_prefix;
		diff->opts.dst_prefix = swap;

	/* do something safe with the pathspec strarray */

	if (!PREFIX_IS_DEFAULT(diff->opts.src_prefix)) {
		git__free(diff->opts.src_prefix);
		diff->opts.src_prefix = NULL;
	}
	if (!PREFIX_IS_DEFAULT(diff->opts.dst_prefix)) {
		git__free(diff->opts.dst_prefix);
		diff->opts.dst_prefix = NULL;
	}
typedef struct {
	git_diff_list *diff;
	git_index *index;
	unsigned int index_pos;
} index_to_tree_info;

static int add_new_index_deltas(
	index_to_tree_info *info,
	const char *stop_path)
{
	int error;
	git_index_entry *idx_entry = git_index_get(info->index, info->index_pos);

	while (idx_entry != NULL &&
		(stop_path == NULL || strcmp(idx_entry->path, stop_path) < 0))
	{
		error = file_delta_new__from_one(
			info->diff, GIT_STATUS_ADDED, idx_entry->mode,
			&idx_entry->oid, idx_entry->path);
		if (error < GIT_SUCCESS)
			return error;

		idx_entry = git_index_get(info->index, ++info->index_pos);
	}

	return GIT_SUCCESS;
}

static int diff_index_to_tree_cb(const char *root, git_tree_entry *tree_entry, void *data)
{
	int error;
	index_to_tree_info *info = data;
	git_index_entry *idx_entry;

	/* TODO: submodule support for GIT_OBJ_COMMITs in tree */
	if (git_tree_entry_type(tree_entry) != GIT_OBJ_BLOB)
		return GIT_SUCCESS;

	error = git_buf_joinpath(&info->diff->pfx, root, git_tree_entry_name(tree_entry));
	if (error < GIT_SUCCESS)
		return error;

	/* create add deltas for index entries that are not in the tree */
	error = add_new_index_deltas(info, info->diff->pfx.ptr);
	if (error < GIT_SUCCESS)
		return error;

	/* create delete delta for tree entries that are not in the index */
	idx_entry = git_index_get(info->index, info->index_pos);
	if (idx_entry == NULL || strcmp(idx_entry->path, info->diff->pfx.ptr) > 0) {
		return file_delta_new__from_one(
			info->diff, GIT_STATUS_DELETED, git_tree_entry_attributes(tree_entry),
			git_tree_entry_id(tree_entry), info->diff->pfx.ptr);
	}

	/* create modified delta for non-matching tree & index entries */
	info->index_pos++;

	if (git_oid_cmp(&idx_entry->oid, git_tree_entry_id(tree_entry)) ||
		idx_entry->mode != git_tree_entry_attributes(tree_entry))
	{
		git_tree_diff_data tdiff;
		tdiff.old_attr = git_tree_entry_attributes(tree_entry);
		tdiff.new_attr = idx_entry->mode;
		tdiff.status   = GIT_STATUS_MODIFIED;
		tdiff.path     = idx_entry->path;
		git_oid_cpy(&tdiff.old_oid, git_tree_entry_id(tree_entry));
		git_oid_cpy(&tdiff.new_oid, &idx_entry->oid);

		error = file_delta_new__from_tree_diff(info->diff, &tdiff);
	}

	return error;

}

int git_diff_index_to_tree(
	git_repository *repo,
	const git_diff_options *opts,
	git_tree *old,
	git_diff_list **diff_ptr)
{
	int error;
	index_to_tree_info info = {0};

	if ((info.diff = git_diff_list_alloc(repo, opts)) == NULL)
		return GIT_ENOMEM;

	if ((error = git_repository_index(&info.index, repo)) == GIT_SUCCESS) {
		error = git_tree_walk(
			old, diff_index_to_tree_cb, GIT_TREEWALK_POST, &info);
		if (error == GIT_SUCCESS)
			error = add_new_index_deltas(&info, NULL);
		git_index_free(info.index);
	}
	git_buf_free(&info.diff->pfx);

	if (error != GIT_SUCCESS)
		git_diff_list_free(info.diff);
	else
		*diff_ptr = info.diff;

	return error;
}

static void setup_xdiff_options(
	git_diff_options *opts, xdemitconf_t *cfg, xpparam_t *param)
{
	memset(cfg, 0, sizeof(xdemitconf_t));
	memset(param, 0, sizeof(xpparam_t));

	cfg->ctxlen =
		(!opts || !opts->context_lines) ? 3 : opts->context_lines;
	cfg->interhunkctxlen =
		(!opts || !opts->interhunk_lines) ? 3 : opts->interhunk_lines;

	if (!opts)
		return;

	if (opts->flags & GIT_DIFF_IGNORE_WHITESPACE)
		param->flags |= XDF_WHITESPACE_FLAGS;
	if (opts->flags & GIT_DIFF_IGNORE_WHITESPACE_CHANGE)
		param->flags |= XDF_IGNORE_WHITESPACE_CHANGE;
	if (opts->flags & GIT_DIFF_IGNORE_WHITESPACE_EOL)
		param->flags |= XDF_IGNORE_WHITESPACE_AT_EOL;
}

	xpparam_t    xdiff_params;
	xdemitconf_t xdiff_config;
	xdemitcb_t   xdiff_callback;
	setup_xdiff_options(&diff->opts, &xdiff_config, &xdiff_params);
	memset(&xdiff_callback, 0, sizeof(xdiff_callback));
	xdiff_callback.outf = diff_output_cb;
	xdiff_callback.priv = &di;

		mmfile_t old_data, new_data;
				old_data.ptr = (char *)git_blob_rawcontent(delta->old_blob);
				old_data.size = git_blob_rawsize(delta->old_blob);
				old_data.ptr = "";
				old_data.size = 0;
				new_data.ptr = (char *)git_blob_rawcontent(delta->new_blob);
				new_data.size = git_blob_rawsize(delta->new_blob);
				new_data.ptr = "";
				new_data.size = 0;
		if (diff->opts.flags & GIT_DIFF_FORCE_TEXT)
			diff->repo, delta, &old_data, &new_data)) < GIT_SUCCESS)
		/* TODO: if ignore_whitespace is set, then we *must* do text
		 * diffs to tell if a file has really been changed.
		 */

		xdl_diff(&old_data, &new_data,
			&xdiff_params, &xdiff_config, &xdiff_callback);
	git_diff_list *diff;
	else if (mode & 0100)
		/* modes in git are not very flexible, so if this bit is set,
		 * we must be dealwith with a 100755 type of file.
		 */
	else if (delta->old_attr != delta->new_attr &&
		delta->old_attr != 0 && delta->new_attr != 0)
	pi.diff     = diff;
	const char *oldpfx = pi->diff->opts.src_prefix;
	const char *newpfx = pi->diff->opts.dst_prefix;
	git_buf_printf(pi->buf, "diff --git %s%s %s%s\n", oldpfx, delta->path, newpfx, newpath);
	pi.diff     = diff;
	xpparam_t xdiff_params;
	xdemitconf_t xdiff_config;
	xdemitcb_t xdiff_callback;

	assert(repo);
	if (options && (options->flags & GIT_DIFF_REVERSE)) {
		git_blob *swap = old_blob;
		old_blob = new_blob;
		new_blob = swap;
	}
	di.line_cb = line_cb;
	setup_xdiff_options(options, &xdiff_config, &xdiff_params);
	memset(&xdiff_callback, 0, sizeof(xdiff_callback));
	xdiff_callback.outf = diff_output_cb;
	xdiff_callback.priv = &di;
	xdl_diff(&old, &new, &xdiff_params, &xdiff_config, &xdiff_callback);