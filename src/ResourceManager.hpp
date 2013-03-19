#ifndef RESOURCE_MANAGER_HPP_INCLUDED
#define RESOURCE_MANAGER_HPP_INCLUDED RESOURCE_MANAGER_HPP_INCLUDED

/// \file ResourceManager.hpp Defines ResourceManager and the function-template loadResource()

#include "exceptions.hpp"
#include "Logfile.hpp"
#include "resfwd.hpp"

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include <string>
#include <unordered_map>


/// \brief Manages resources and ensures that they are loaded only once.
///
/// Use this class to load any classes from SFML which have a loadFromFile() method. Other
/// resource types are supported by specializing load() or by using setResourceNotFoundCallback().
/// This class holds \c tr1::weak_ptrs to the resources, so they are automatically deleted,
/// if they are not used. Additionaly, you can ensure that a particular Resource won't be
/// unloaded by calling keepLoaded().
/// If you don't set the ResourceNotFoundCallback, no resources can be loaded. If you want
/// to use the default method, call useDefaultCallback().
template<typename ResT>
class ResourceManager: private boost::noncopyable {
public:
    typedef ResourceTraits<ResT> Traits;
    typedef typename Traits::Ptr Ptr;
    typedef typename Traits::WeakPtr WeakPtr;
    typedef typename Traits::Resource Resource;

    typedef boost::function<void(Resource&, std::string const&)>
        ResourceNotFoundCallback;

	ResourceManager();

	/// \brief Loads \a name if neccessary and returns it.
    /// 
    /// If no callback is set, behaves like get().
	/// \returns The resource \a name.
	/// \throws ResourceLoadError or derived normally.
    ///         In fact anything thrown by the ResourceNotFoundCallback.
	Ptr request(std::string const& name);

	/// \brief As opposed to request(), this function does not
    ///        try to load the resource \a name.
	/// \returns The resource \a name.
	/// \throws ResourceError if \a name is not loaded.
	Ptr get(std::string const& name);

    /// \brief As get(), but returns nullptr instead of throwing.
    Ptr tryGet(std::string const& name);

	/// \brief Makes \a res available (by request() and get()) as \a name
	void insert(std::string const& name, Ptr res);

	/// \brief \a name will not be deleted, if it's unused.
	///
	/// If \name is not already loaded, this is done by this function.
	/// \throws ResourceLoadError if \a name was not loaded and loading failed.
	Ptr keepLoaded(std::string const& name);

	/// \brief If \a enable is true, resources will not be deleted automatically.
	///
	/// I.e. if you call request() it behaves like you called keepLoaded() before.
	///
	/// Note that calling with \a enable = false does not release any resources, which are kept.
	void keepAllLoaded(bool enable = true);

	/// \brief Use this to reenable auto-deletion of a file that was previously kept.
	///
	/// You should use this when don't need a resource on which you called keepLoaded().
	/// Otherwise, a warning is written to the logfile in the destructor.
	void release(std::string const& name);

	/// \brief Reenables auto-deletion for all previously kept resources.
	void releaseAll();

	/// \brief Cleans up internal data-structures to gain you some additional bytes.
	///
	/// For each deleted resource, an expired \c tr1::weak_ptr remains. This function removes them.
	/// You might want to call this function, after a lots of resources got released.
	void tidy();

    /// \brief Calls releaseAll() and then tidy()
    void purge();

    /// \brief Set the function for loading resources.
    ResourceNotFoundCallback setResourceNotFoundCallback(
        ResourceNotFoundCallback const&);

    /// \brief Sets the appropiate specialization of loadResource as callback.
    void useDefaultCallback();

	~ResourceManager();

private:
	typedef std::unordered_map<std::string, WeakPtr> resMap_t;
	resMap_t m_resMap;
	typedef std::unordered_map<std::string, Ptr> keptMap_t;
	keptMap_t m_kept;

    ResourceNotFoundCallback m_callback;
	bool m_keepAll;
};

/// \brief Loads \a resource using it's method <c>LoadFromFile(\a name)</c>.
///
/// This function is designed to work with classes from SFML (Image, Font, SoundBuffer).
/// Therefore when loading fails, \c LoadFromFile() must return false, and true on success.
/// Overload this function, to be able to work with custom resources.
/// \throws ResourceLoadError if loading failed.
/// \see ConfigResource.hpp
template<typename ResT>
void loadResource(ResT& resource, std::string const& name);

template<typename T>
ResourceManager<T>& resMng()
{
	static ResourceManager<T> instance;
	return instance;
}


#include "ResourceManager.inl"

#endif
