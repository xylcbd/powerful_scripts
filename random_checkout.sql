SELECT url FROM images
 WHERE platform=2 AND id >= (SELECT FLOOR( RAND() * ((SELECT MAX(id) FROM images)-(SELECT MIN(id) FROM images)) + (SELECT MIN(id) FROM images)))    
 ORDER BY id LIMIT 0,200;
