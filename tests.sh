for file in Tests/*.scope; do
	if [ -f "$file" ]; then
		echo "\033[0mTesting \`$file\`\033[0;31m"
		SCOPE_OUT=$(./scope $file);
		EXPECTED=$(cat $file.txt);

		if [ "$SCOPE_OUT" != "$EXPECTED" ]; then
			echo "Test Failed.\033[0m"
		fi
	fi
done
echo "\033[0mDone!"