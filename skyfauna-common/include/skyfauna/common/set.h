// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#ifndef SKYFAUNA_SET_H_
#define SKYFAUNA_SET_H_

#include <skyfauna/common/util.h>

#include <array>
#include <type_traits>
#include <utility>

namespace skyfauna::common {
template<typename Key, typename Hasher = Hash<Key>>
requires IsHashableV<Key>
class HashSet {
public:
	using key_type = Key;
	using hasher = Hasher;

	using data_type = std::vector<std::pair<key_type, std::size_t>>;
	using iterator = data_type::iterator;
	using const_iterator = data_type::const_iterator;
	using reverse_iterator = data_type::reverse_iterator;
	using const_reverse_iterator = data_type::const_reverse_iterator;

public:
	HashSet() = default;
	HashSet(const std::vector<key_type>& vec)
	 : m_Data(vec.size())
	{
		std::transform(vec.begin(), vec.end(), m_Data.begin(),
			[](const key_type& key) -> std::decay_t<decltype(m_Data[0])> {
				return { key, hasher{}(key) }; 
			});
		std::sort(m_Data.begin(), m_Data.end(),
			[](const auto& lhs, const auto& rhs) {
				return lhs.second < rhs.second;
			});
	}

	void clear() { m_Data.clear(); };

	 std::size_t HashAt(std::size_t n) const { return m_Data[n].second; }
	 const key_type& KeyAt(std::size_t n) const { return m_Data[n].first; }

	 iterator begin() { return m_Data.begin(); }
	 iterator end() { return m_Data.end(); }
	 const_iterator begin() const { return m_Data.begin(); }
	 const_iterator end() const { return m_Data.end(); }
	 const_iterator cbegin() const { return m_Data.cbegin(); }
	 const_iterator cend() const { return m_Data.cend(); }
	 reverse_iterator rbegin() { return m_Data.rbegin(); }
	 reverse_iterator rend() { return m_Data.rend(); }
	 const_reverse_iterator rbegin() const { return m_Data.rbegin(); }
	 const_reverse_iterator rend() const { return m_Data.rend(); }
	 const_reverse_iterator crbegin() const { return m_Data.crbegin(); }
	 const_reverse_iterator crend() const { return m_Data.crend(); }

	// TODO: Binary search
	iterator Find(std::size_t hash) {
		return std::find_if(begin(), end(),
				   [hash](const auto& lhs){ return lhs.second == hash; });
	}
	iterator Find(const std::string_view str) {
		return Find(hasher{}(str));
	}
	const_iterator Find(const std::size_t hash) const {
		return std::find_if(cbegin(), cend(),
				   [hash](const auto& lhs){ return lhs.second == hash; });
	}
	const_iterator Find(const std::string_view str) const {
		return Find(hasher{}(str));
	}

private:
	data_type m_Data;
};

template<typename Key, std::size_t N, typename Hasher = Hash<Key>>
requires IsHashableV<Key>
class FixedHashSet {
public:
	using key_type = Key;
	using hasher = Hasher;

	using pair_type = std::pair<key_type, std::size_t>;
	using data_type = std::array<pair_type, N>;
	using iterator = data_type::iterator;
	using const_iterator = data_type::const_iterator;
	using reverse_iterator = data_type::reverse_iterator;
	using const_reverse_iterator = data_type::const_reverse_iterator;

public:
	constexpr FixedHashSet() noexcept = default;
	template<typename InputIt, typename End>
	requires std::input_iterator<InputIt> &&
		std::sentinel_for<End, InputIt> &&
		std::same_as<std::iter_value_t<InputIt>, key_type>
	constexpr FixedHashSet(InputIt it, End end) {
		std::transform(it, end, m_Data.begin(),
			[](const key_type& key) -> pair_type {
				return { key, hasher{}(key) }; 
			});
		std::sort(m_Data.begin(), m_Data.end(),
			[](const auto& lhs, const auto& rhs) {
				return lhs.second < rhs.second;
			});
	}
	template<typename InputIt, typename End>
	requires std::input_iterator<InputIt> &&
		std::sentinel_for<End, InputIt> &&
		std::same_as<std::iter_value_t<InputIt>, pair_type>
	constexpr FixedHashSet(InputIt it, End end)
	 : m_Data(it, end)
	{
		std::sort(m_Data.begin(), m_Data.end(),
			[](const auto& lhs, const auto& rhs) {
				return lhs.second < rhs.second;
			});
	}
	constexpr FixedHashSet(const std::array<key_type, N>& arr) noexcept {
		std::transform(arr.begin(), arr.end(), m_Data.begin(),
			[](const key_type& key) -> pair_type {
				return { key, hasher{}(key) }; 
			});
		std::sort(m_Data.begin(), m_Data.end(),
			[](const auto& lhs, const auto& rhs) {
				return lhs.second < rhs.second;
			});
	}

	constexpr std::size_t HashAt(std::size_t n) const noexcept { return m_Data[n].second; }
	constexpr const key_type& KeyAt(std::size_t n) const noexcept { return m_Data[n].first; }

	constexpr iterator begin() noexcept { return m_Data.begin(); }
	constexpr iterator end() noexcept { return m_Data.end(); }
	constexpr const_iterator begin() const noexcept { return m_Data.begin(); }
	constexpr const_iterator end() const noexcept { return m_Data.end(); }
	constexpr const_iterator cbegin() const noexcept { return m_Data.cbegin(); }
	constexpr const_iterator cend() const noexcept { return m_Data.cend(); }
	constexpr reverse_iterator rbegin() noexcept { return m_Data.rbegin(); }
	constexpr reverse_iterator rend() noexcept { return m_Data.rend(); }
	constexpr const_reverse_iterator rbegin() const noexcept { return m_Data.rbegin(); }
	constexpr const_reverse_iterator rend() const noexcept { return m_Data.rend(); }
	constexpr const_reverse_iterator crbegin() const noexcept { return m_Data.crbegin(); }
	constexpr const_reverse_iterator crend() const noexcept { return m_Data.crend(); }

	// TODO: Binary search
	constexpr iterator Find(std::size_t hash) noexcept {
		return std::find_if(begin(), end(),
				   [hash](const auto& lhs){ return lhs.second == hash; });
	}
	constexpr iterator Find(const std::string_view str) noexcept {
		return Find(hasher{}(str));
	}
	constexpr const_iterator Find(const std::size_t hash) const noexcept {
		return std::find_if(cbegin(), cend(),
				   [hash](const auto& lhs){ return lhs.second == hash; });
	}
	constexpr const_iterator Find(const std::string_view str) const noexcept {
		return Find(hasher{}(str));
	}

private:
	data_type m_Data;
};

constexpr std::array<std::string_view, 1> arr{ std::string_view("") };
constexpr FixedHashSet<std::string_view, 1> hs(arr);
static_assert(hs.HashAt(0) == Hash<std::string_view>{}(arr[0]));
}

#endif

