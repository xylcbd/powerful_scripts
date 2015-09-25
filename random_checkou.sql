SELECT url FROM images
 WHERE platform=2 AND id >= (SELECT FLOOR( RAND() * ((SELECT MAX(id) FROM images_20150909_feeds)-(SELECT MIN(id) FROM images_20150909_feeds)) + (SELECT MIN(id) FROM images_20150909_feeds)))    
 ORDER BY id LIMIT 0,200;
