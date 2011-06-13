#include "split.h"

std::vector <std::string> Split( const std::string& _separator, std::string _string )
{

	std::vector <std::string> strVec;

	size_t position;
	
	// Search for separator.
	position = _string.find_first_of( _separator );
	
	// We will loop true this until there are no separators left
	// in _string
	while ( position != _string.npos ) {
	
		// This thing here checks that we dont push empty strings
		// to the array
		if ( position != 0 )
			strVec.push_back( _string.substr( 0, position ) );

		// When the cutt part is pushed into the array we
		// remove it and the separator from the _string
		_string.erase( 0, position + _separator.length() );

		// And the we look for a new place for the _separator
		position = _string.find_first_of( _separator );
	}

	// We will push the rest of the stuff in to the array
	if ( _string.empty() == false ) {
		strVec.push_back( _string );
	}

	// Then we'll just return the array
	return strVec;
}

std::vector <std::string> Split( const std::string& _separator, std::string _string, int _limit )
{

	std::vector <std::string> strVec;

	size_t position;
	position = _string.find_first_of( _separator );

	// The only diffrence is here
	int count = 0;	

	// and in this while loop the count <= _limit
	while ( position != _string.npos &&  count < _limit ) {
	
		if ( position != 0 )
			strVec.push_back( _string.substr( 0, position ) );

		_string.erase( 0, position + _separator.length() );

		position = _string.find_first_of( _separator );

		// And here
		count++;
	}
	if ( _string.empty() == false ) {
		strVec.push_back( _string );
	}

	return strVec;
}

std::vector <std::string> SplitC( const std::string& _separator, std::string _string, int &length )
{

	// length counts the separators.
	// Example:
		/*std::vector <std::string> bbb;
		int h;
		ostringstream j;

		bbb=SplitC(" ","ah boo cee h",h);
		j << h;*/
	//  j would return 3
	/////////////////////////////////////


	std::vector <std::string> strVec;

	size_t position;
	int Count=0;

	// Search for separator.
	position = _string.find_first_of( _separator );

	// We will loop true this until there are no separators left
	// in _string
	while ( position != _string.npos ) {

		// This thing here checks that we dont push empty strings
		// to the array
		if ( position != 0 )
			strVec.push_back( _string.substr( 0, position ) );

		// When the cutt part is pushed into the array we
		// remove it and the separator from the _string
		_string.erase( 0, position + _separator.length() );

		// And the we look for a new place for the _separator
		position = _string.find_first_of( _separator );
		Count++;
	}

	// We will push the rest of the stuff in to the array
	if ( _string.empty() == false ) {
		strVec.push_back( _string );
	}

	length=Count;

	// Then we'll just return the array
	return strVec;
}
