/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2015 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "GOrgueZipFormat.h"

#include <wx/defs.h>

void GOZipLocalHeader::Swap()
{
	signature =  wxUINT32_SWAP_ON_BE( signature );
	version_extract =  wxUINT16_SWAP_ON_BE( version_extract );
	flags =  wxUINT16_SWAP_ON_BE( flags );
	compression =  wxUINT16_SWAP_ON_BE( compression );
	modification_time =  wxUINT16_SWAP_ON_BE( modification_time );
	modification_date =  wxUINT16_SWAP_ON_BE( modification_date );
	crc =  wxUINT32_SWAP_ON_BE( crc );
	compressed_size =  wxUINT32_SWAP_ON_BE( compressed_size );
	uncompressed_size =  wxUINT32_SWAP_ON_BE( uncompressed_size );
	name_length =  wxUINT16_SWAP_ON_BE( name_length );
	extra_length =  wxUINT16_SWAP_ON_BE( extra_length );
}

void GOZipCentralHeader::Swap()
{
	signature =  wxUINT32_SWAP_ON_BE( signature );
	version_creator =  wxUINT16_SWAP_ON_BE( version_creator );
	version_extract =  wxUINT16_SWAP_ON_BE( version_extract );
	flags =  wxUINT16_SWAP_ON_BE( flags );
	compression =  wxUINT16_SWAP_ON_BE( compression );
	modification_time =  wxUINT16_SWAP_ON_BE( modification_time );
	modification_date =  wxUINT16_SWAP_ON_BE( modification_date );
	crc =  wxUINT32_SWAP_ON_BE( crc );
	compressed_size =  wxUINT32_SWAP_ON_BE( compressed_size );
	uncompressed_size =  wxUINT32_SWAP_ON_BE( uncompressed_size );
	name_length =  wxUINT16_SWAP_ON_BE( name_length );
	extra_length =  wxUINT16_SWAP_ON_BE( extra_length );
	comment_length =  wxUINT16_SWAP_ON_BE( comment_length );
	disk_number =  wxUINT16_SWAP_ON_BE( disk_number );
	internal_attributes =  wxUINT16_SWAP_ON_BE( internal_attributes );
	external_attributes =  wxUINT32_SWAP_ON_BE( external_attributes );
	offset =  wxUINT32_SWAP_ON_BE( offset );
}

void GOZipHeaderExtraRecord::Swap()
{
	type =  wxUINT16_SWAP_ON_BE( type );
	size =  wxUINT16_SWAP_ON_BE( size );
}

void GOZipEnd64Record::Swap()
{
	signature =  wxUINT32_SWAP_ON_BE( signature );
	size =  wxUINT64_SWAP_ON_BE( size );
	version_creator =  wxUINT16_SWAP_ON_BE( version_creator );
	version_extract =  wxUINT16_SWAP_ON_BE( version_extract );
	current_disk =  wxUINT32_SWAP_ON_BE( current_disk );
	directory_disk =  wxUINT32_SWAP_ON_BE( directory_disk );
	entry_count_disk =  wxUINT64_SWAP_ON_BE( entry_count_disk );
	entry_count =  wxUINT64_SWAP_ON_BE( entry_count );
	directory_size =  wxUINT64_SWAP_ON_BE( directory_size );
	directory_offset =  wxUINT64_SWAP_ON_BE( directory_offset );
}

void GOZipEnd64BlockHeader::Swap()
{
	header_id = wxUINT16_SWAP_ON_BE( header_id );
	size = wxUINT32_SWAP_ON_BE( size );
}

void  GOZipEnd64Locator::Swap()
{
	signature =  wxUINT32_SWAP_ON_BE( signature );
	end_record_disk =  wxUINT32_SWAP_ON_BE( end_record_disk );
	end_record_offset =  wxUINT64_SWAP_ON_BE( end_record_offset );
	disk_count =  wxUINT32_SWAP_ON_BE( disk_count );
}

void GOZipEndRecord::Swap()
{
	signature =  wxUINT32_SWAP_ON_BE( signature );
	current_disk =  wxUINT16_SWAP_ON_BE( current_disk );
	directory_disk =  wxUINT16_SWAP_ON_BE( directory_disk );
	entry_count_disk =  wxUINT16_SWAP_ON_BE( entry_count_disk );
	entry_count =  wxUINT16_SWAP_ON_BE( entry_count );
	directory_size =  wxUINT32_SWAP_ON_BE( directory_size );
	directory_offset =  wxUINT32_SWAP_ON_BE( directory_offset );
	comment_len =  wxUINT16_SWAP_ON_BE( comment_len );
}
