CREATE EXTENSION IF NOT EXISTS CITEXT;

DROP SCHEMA IF EXISTS realworld CASCADE;
CREATE SCHEMA IF NOT EXISTS realworld;

CREATE TABLE IF NOT EXISTS realworld.users (
	user_id SERIAL,
	username CITEXT NOT NULL,
	email VARCHAR(255) NOT NULL,
	password_hash VARCHAR(255) NOT NULL,
	bio TEXT,
	image VARCHAR(255),
	CONSTRAINT pk_users PRIMARY KEY(user_id),
	CONSTRAINT uniq_username UNIQUE(username),
	CONSTRAINT uniq_email UNIQUE(email)
);

CREATE TABLE IF NOT EXISTS realworld.articles (
	article_id SERIAL,
	title VARCHAR(255) NOT NULL,
	slug VARCHAR(255) NOT NULL,
	description TEXT NOT NULL,
	body TEXT NOT NULL,
	author_id INT NOT NULL,
	created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
	updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
	CONSTRAINT pk_articles PRIMARY KEY(article_id),
	CONSTRAINT fk_article_author FOREIGN KEY(author_id) REFERENCES realworld.users(user_id),
	CONSTRAINT uniq_slug UNIQUE(slug)
);

CREATE TABLE IF NOT EXISTS realworld.tags (
	tag_id SERIAL,
	name VARCHAR(255),
	CONSTRAINT pk_tags PRIMARY KEY(tag_id),
	CONSTRAINT uniq_name UNIQUE(name)
);

CREATE TABLE IF NOT EXISTS realworld.article_tags (
	article_id INT NOT NULL,
	tag_id INT NOT NULL,
	CONSTRAINT pk_article_tags PRIMARY KEY(article_id, tag_id),
	CONSTRAINT fk_article FOREIGN KEY(article_id) REFERENCES realworld.articles(article_id) ON DELETE CASCADE,
	CONSTRAINT fk_tag FOREIGN KEY(tag_id) REFERENCES realworld.tags(tag_id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS realworld.favorites (
	user_id INT NOT NULL,
	article_id INT NOT NULL,
	CONSTRAINT pk_favorites PRIMARY KEY(user_id, article_id),
	CONSTRAINT fk_user FOREIGN KEY(user_id) REFERENCES realworld.users(user_id),
	CONSTRAINT fk_article FOREIGN KEY(article_id) REFERENCES realworld.articles(article_id)
);

CREATE TABLE IF NOT EXISTS realworld.followers (
	follower INT NOT NULL,
	followed INT NOT NULL,
	CONSTRAINT pk_followers PRIMARY KEY(follower, followed),
	CONSTRAINT fk_follower FOREIGN KEY(follower) REFERENCES realworld.users(user_id),
	CONSTRAINT fk_followed FOREIGN KEY(followed) REFERENCES realworld.users(user_id)
);

CREATE TABLE IF NOT EXISTS realworld.comments (
	comment_id SERIAL,
	author_id INT NOT NULL,
	article_id INT NOT NULL,
	body VARCHAR(16384) NOT NULL,
	created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
	updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
	CONSTRAINT pk_comments PRIMARY KEY(comment_id),
	CONSTRAINT fk_article FOREIGN KEY(article_id) REFERENCES realworld.articles(article_id) ON DELETE CASCADE,
	CONSTRAINT fk_author FOREIGN KEY(author_id) REFERENCES realworld.users(user_id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_users_username ON realworld.users(username);
CREATE INDEX IF NOT EXISTS idx_articles_slug ON realworld.articles(slug);
CREATE INDEX IF NOT EXISTS idx_articles_author_id ON realworld.articles(author_id);

CREATE TYPE realworld.realworld_user AS
(
	user_id INT,
	username CITEXT,
	email VARCHAR(255),
	password_hash VARCHAR(255),
	bio TEXT,
	image VARCHAR(255)
);

CREATE TYPE realworld.profile AS
(
	username CITEXT,
	bio TEXT,
	image VARCHAR(255),
	following BOOL
);

CREATE TYPE realworld.realworld_comment AS
(
	comment_id INT,
	created_at TIMESTAMP WITH TIME ZONE,
	updated_at TIMESTAMP WITH TIME ZONE,
	body VARCHAR(16384),
	author realworld.profile
);

CREATE TYPE realworld.article_with_author_profile AS
(
	article_id INT,
	title VARCHAR(255),
	slug VARCHAR(255),
	description TEXT,
	body TEXT,
	created_at TIMESTAMP WITH TIME ZONE,
	updated_at TIMESTAMP WITH TIME ZONE,
	tag_list VARCHAR(255)[],
	favorited BOOL,
	favorites_count BIGINT,
	author realworld.profile
);

CREATE OR REPLACE FUNCTION realworld.add_comment_to_article(
	_slug VARCHAR(255),
	_body VARCHAR(16384),
	_user_id INT)
    RETURNS INT
AS $$
DECLARE
	_comment_id INT;
BEGIN
	INSERT INTO
		realworld.comments (body, author_id, article_id)
	VALUES
		(_body, _user_id, (SELECT article_id FROM realworld.articles WHERE slug = _slug))
	RETURNING 
		comment_id
	INTO 
		_comment_id;

	RETURN _comment_id;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION realworld.add_new_article(
	_title VARCHAR(255),
	_slug VARCHAR(255),
	_description TEXT,
	_body TEXT,
	_author_id INT,
	_tag_list VARCHAR(255)[])
    RETURNS INT
AS $$
DECLARE
	_new_article_id INT;
BEGIN
	INSERT INTO
		realworld.articles (title, slug, description, body, author_id)
	VALUES
		(_title, _slug, _description, _body, _author_id)
	RETURNING 
		article_id INTO _new_article_id;

	INSERT INTO 
		realworld.tags(name)
	SELECT 
		unnest(_tag_list)
	ON CONFLICT DO NOTHING;

	INSERT INTO 
		realworld.article_tags (article_id, tag_id)
	SELECT 
		_new_article_id, tag_ids.tag_id
	FROM (
		SELECT 
			tag_id
		FROM 
			realworld.tags
		WHERE 
			name = ANY (_tag_list)) AS tag_ids;

	RETURN _new_article_id;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION realworld.add_new_user(
	_username CITEXT,
	_email VARCHAR(255),
	_hash VARCHAR(255))
    RETURNS INT
AS $$
DECLARE
	_user_id INT;
BEGIN
	INSERT INTO 
		realworld.users(username, email, password_hash)
	VALUES
		(_username, _email, _hash)
	RETURNING 
		user_id
	INTO 
		_user_id;

	RETURN _user_id;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION realworld.delete_article_by_slug(
	_slug VARCHAR(255),
	_author_id INT)
    RETURNS VOID
AS $$
DECLARE 
	_deleted_rows_count INT;
BEGIN
	DELETE FROM
		realworld.articles
	WHERE
		slug = _slug AND
		author_id = _author_id;	
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION realworld.delete_comment(
	_comment_id INT,
	_slug VARCHAR(255),
	_author_id INT)
    RETURNS VOID
AS $$
DECLARE
	_deleted_rows_count INT;
BEGIN
	DELETE FROM
		realworld.comments
	WHERE
		comment_id = _comment_id AND
		author_id = _author_id AND
		article_id = (SELECT article_id FROM realworld.articles WHERE slug = _slug);
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION realworld.is_comment_exist(
	_comment_id INT,
	_slug VARCHAR(255),
	_author_id INT)
		RETURNS BOOL
AS $$
BEGIN
	RETURN
	EXISTS(
		SELECT
			1
		FROM
			realworld.comments
		WHERE
			comment_id = _comment_id AND
			author_id = _author_id AND
			article_id = (SELECT article_id FROM realworld.articles WHERE slug = _slug)
	);
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION realworld.favorite_article(
	_slug VARCHAR(255),
	_user_id INT)
    RETURNS VOID
AS $$
BEGIN
	INSERT INTO
		realworld.favorites (user_id, article_id)
	VALUES
		(_user_id, (SELECT article_id FROM realworld.articles WHERE slug = _slug));
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION realworld.follow(
	_follower INT,
	_followed INT)
    RETURNS VOID
AS $$
BEGIN
	INSERT INTO 
		realworld.followers(follower, followed)
	VALUES
		(_follower, _followed);
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION realworld.get_article_id_by_slug(
	_slug VARCHAR(255))
    RETURNS SETOF INT 
AS $$
DECLARE
	_article_id INT;
BEGIN
	RETURN QUERY
	SELECT
		article_id
	FROM
		realworld.articles
	WHERE
		slug = _slug;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION realworld.get_article_tag_list(
	_article_id INT)
    RETURNS SETOF VARCHAR(255) 
AS $$
BEGIN
RETURN QUERY
	SELECT 
		t.name
	FROM 
		realworld.article_tags AS at
	INNER JOIN 
		realworld.tags AS t ON t.tag_id = at.tag_id
	WHERE 
		article_id = _article_id
	ORDER BY
		t.name ASC;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION realworld.get_article_with_author_profile(
	_id INT,
	_follower_id INT = NULL)
    RETURNS SETOF realworld.article_with_author_profile 
AS $$
BEGIN
	RETURN QUERY
	SELECT
		article_id,
		title,
		slug,
		description,	
		body,
		created_at,
		updated_at,
		ARRAY(SELECT * FROM realworld.get_article_tag_list(_id))::VARCHAR(255)[],
		realworld.is_favorited_article(_id, _follower_id),
		(SELECT COUNT(*) FROM realworld.favorites WHERE article_id = _id),
		realworld.get_profile(author_id, _follower_id)
	FROM
		realworld.articles
	WHERE
		article_id = _id;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION realworld.get_article_with_author_profile_by_slug(
	_slug VARCHAR(255),
	_follower_id INT = NULL)
    RETURNS SETOF realworld.article_with_author_profile 
AS $$
DECLARE
	_article_id INT;
BEGIN
	SELECT article_id FROM realworld.articles WHERE slug = _slug INTO _article_id;
	
	RETURN QUERY
	SELECT
		article_id,
		title,
		slug,
		description,	
		body,
		created_at,
		updated_at,
		ARRAY(SELECT * FROM realworld.get_article_tag_list(_article_id))::VARCHAR(255)[],
		realworld.is_favorited_article(_article_id, _follower_id),
		(SELECT COUNT(*) FROM realworld.favorites WHERE article_id = _article_id),
		realworld.get_profile(author_id, _follower_id)
	FROM
		realworld.articles
	WHERE
		article_id = _article_id;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION realworld.get_articles_with_author_profile(
	_tag VARCHAR(255) = NULL,
	_author_username CITEXT = NULL,
	_favorited_by_user CITEXT = NULL,
	_user_id INT = NULL,
	_limit INT = 20,
	_offset INT = 0)
    RETURNS SETOF realworld.article_with_author_profile 
AS $$
BEGIN
	RETURN QUERY
	SELECT 
		article_id,
		title,
		slug,
		description,	
		body,
		created_at,
		updated_at,
		ARRAY(SELECT * FROM realworld.get_article_tag_list(article_id))::VARCHAR(255)[],
		realworld.is_favorited_article(article_id),
		(SELECT 
			COUNT(*) 
		FROM 
			realworld.favorites 
		WHERE 
			realworld.favorites.article_id = realworld.articles.article_id),
		realworld.get_profile(realworld.articles.author_id, _user_id)
	FROM 
		realworld.articles
	INNER JOIN 
		realworld.users ON realworld.articles.author_id = realworld.users.user_id
	WHERE 
		(_tag IS NULL OR
		article_id IN (
			SELECT 
				article_id 
			FROM 
				realworld.article_tags
			INNER JOIN 
				realworld.tags ON realworld.article_tags.tag_id = realworld.tags.tag_id 
			WHERE 
				realworld.tags.name = _tag)) AND	
		(_author_username IS NULL OR realworld.users.username = _author_username) AND
		(_favorited_by_user IS NULL OR
		article_id IN (
			SELECT 
				article_id 
			FROM 
				realworld.favorites
			INNER JOIN 
				realworld.users ON realworld.favorites.user_id = realworld.users.user_id
			WHERE 
				realworld.users.username = _favorited_by_user))
	ORDER BY realworld.articles.created_at DESC
	LIMIT 
		(CASE WHEN _limit IS NULL THEN 20 ELSE _limit END)
	OFFSET 
		(CASE WHEN _offset IS NULL THEN 0 ELSE _offset END);
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION realworld.get_comment(
	_comment_id INT,
	_user_id INT = NULL)
    RETURNS SETOF realworld.realworld_comment 
AS $$
BEGIN
	RETURN QUERY
	SELECT
		comments.comment_id,
		comments.created_at,
		comments.updated_at,
		comments.body,
		realworld.get_profile(comments.author_id, _user_id)
	FROM
		realworld.comments AS comments
	INNER JOIN 
		realworld.articles AS articles ON articles.article_id = comments.article_id
	WHERE
		comments.comment_id = _comment_id;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION realworld.get_comments_from_article(
	_slug VARCHAR(255),
	_user_id INT = NULL)
    RETURNS SETOF realworld.realworld_comment 
AS $$
BEGIN
	RETURN QUERY
	SELECT
		comments.comment_id,
		comments.created_at,
		comments.updated_at,
		comments.body,
		realworld.get_profile(comments.author_id, _user_id)
	FROM
		realworld.comments AS comments
	INNER JOIN 
		realworld.articles AS articles ON articles.article_id = comments.article_id
	WHERE
		articles.slug = _slug;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION realworld.get_feed(
	_user_id INT,
	_limit INT = 20,
	_offset INT = 0)
    RETURNS SETOF realworld.article_with_author_profile 
AS $$
BEGIN
	RETURN QUERY
	SELECT 
		article_id,
		title,
		slug,
		description,	
		body,
		created_at,
		updated_at,
		ARRAY(SELECT * FROM realworld.get_article_tag_list(article_id))::VARCHAR(255)[],
		realworld.is_favorited_article(article_id),
		(SELECT COUNT(*) FROM realworld.favorites WHERE article_id = article_id),
		realworld.get_profile(author_id, _user_id)
	FROM 
		realworld.articles
	WHERE
		author_id IN (
			SELECT followed FROM realworld.followers WHERE follower = _user_id
		)		
	ORDER BY created_at DESC
	LIMIT
		_limit
	OFFSET
		_offset;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION realworld.get_profile(
	_id INT,
	_follower_id INT = NULL)
    RETURNS SETOF realworld.profile 
AS $$
BEGIN
RETURN QUERY
	SELECT 
		username, 
		bio, 
		image, 
		CASE WHEN _follower_id = NULL THEN
			FALSE
		ELSE
			 realworld.is_following(_follower_id, _id)
		END
	FROM 
		realworld.users
	WHERE 
		user_id = _id;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION realworld.get_profile_by_username(
	_username CITEXT,
	_follower_id INT = NULL)
    RETURNS SETOF realworld.profile 
AS $$
BEGIN
	RETURN QUERY
	SELECT 
		username, 
		bio, 
		image, 
		CASE WHEN _follower_id = NULL THEN
			FALSE
		ELSE
			 realworld.is_following(_follower_id, user_id)
		END
	FROM 
		realworld.users
	WHERE 
		username = _username;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION realworld.get_tags()
    RETURNS SETOF VARCHAR(255) 
AS $$
BEGIN
	RETURN QUERY
	SELECT
		name
	FROM
		realworld.article_tags AS at
	INNER JOIN
		realworld.tags AS t ON t.tag_id = at.tag_id;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION realworld.get_user_by_email(
	_email VARCHAR(255))
    RETURNS SETOF realworld.realworld_user 
AS $$
BEGIN
	RETURN QUERY
	SELECT 
		user_id, 
		username, 
		email, 
		password_hash,		
		bio, 
		image
	FROM 
		realworld.users
	WHERE 
		email = _email;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION realworld.get_user_by_id(
	_user_id INT)
    RETURNS SETOF realworld.realworld_user 
AS $$
BEGIN
	RETURN QUERY
	SELECT 
		user_id, 
		username, 
 		email, 
		password_hash,		
		bio, 
		image
	FROM 
		realworld.users
	WHERE 
		user_id = _user_id;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION realworld.get_user_by_username(
	_username CITEXT)
    RETURNS SETOF realworld.realworld_user 
AS $$
BEGIN
	RETURN QUERY
	SELECT 
		user_id, 
		username, 
		email, 
		password_hash,		
		bio, 
		image
	FROM 
		realworld.users
	WHERE 
		username = _username;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION realworld.is_favorited_article(
	_article_id INT,
	_user_id INT = NULL)
    RETURNS BOOL
AS $$
BEGIN
	RETURN EXISTS (
		SELECT 
			1 
		FROM 
			realworld.favorites
		WHERE user_id = _user_id AND article_id = _article_id
	);
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION realworld.is_following(
	_follower INT,
	_followed INT)
    RETURNS BOOL
AS $$
BEGIN
	RETURN EXISTS (
		SELECT 
			1 
		FROM 
			realworld.followers
		WHERE follower = _follower AND followed = _followed
	);
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION realworld.unfavorite_article(
	_slug VARCHAR(255),
	_user_id INT)
    RETURNS VOID
AS $$
BEGIN
	DELETE FROM
		realworld.favorites
	WHERE
		user_id = _user_id AND 
		article_id = (SELECT article_id FROM realworld.articles WHERE slug = _slug);
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION realworld.unfollow(
	_follower INT,
	_followed INT)
    RETURNS VOID
AS $$
BEGIN
	DELETE FROM
		realworld.followers
	WHERE 
		follower = _follower AND 
		followed = _followed;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION realworld.update_article_by_slug(
	_old_slug VARCHAR(255),
	_author_id INT,
	_title VARCHAR(255) = NULL,
	_new_slug VARCHAR(255) = NULL,
	_description TEXT = NULL,
	_body TEXT = NULL)
    RETURNS SETOF INT 
AS $$
BEGIN
	RETURN QUERY
	UPDATE
		realworld.articles
	SET 
		title = COALESCE(_title, title),
		slug = COALESCE(_new_slug, slug),
		description = COALESCE(_description, description),
		body = COALESCE(_body, body),
		updated_at = NOW()
	WHERE
		slug = _old_slug AND
		author_id = _author_id
	RETURNING 
		article_id;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION realworld.update_user_by_id(
	_user_id INT,
	_username CITEXT = NULL,
	_email VARCHAR(255) = NULL,
	_password_hash VARCHAR(255) = NULL,
	_bio TEXT = NULL,
	_image VARCHAR(255) = NULL)
    RETURNS SETOF realworld.realworld_user 
AS $$
BEGIN
	RETURN QUERY
	UPDATE 
		realworld.users 
	SET
		username = COALESCE(_username, username),
		email = COALESCE(_email, email),
		password_hash = COALESCE(_password_hash, password_hash),
		bio = COALESCE(_bio, bio),
		image = COALESCE(_image, image)
	WHERE
		user_id = _user_id
	RETURNING 
		user_id, 
		username, 
 		email, 
		password_hash,		
		bio, 
		image;
END;
$$ LANGUAGE plpgsql;