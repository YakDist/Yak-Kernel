ld --verbose | sed -n '/^==/,/^==/p' | sed '1d;$d'
