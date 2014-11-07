#include <unity/scopes/internal/BufferedSearchReplyImpl.h>

namespace unity
{

namespace scopes
{

namespace internal
{

BufferedSearchReplyImpl::BufferedSearchReplyImpl(unity::scopes::SearchReplyProxy const& upstream)
    : upstream_(upstream)
{
}

void BufferedSearchReplyImpl::register_departments(Department::SCPtr const& parent)
{
    upstream_->register_departments(parent);
}

Category::SCPtr BufferedSearchReplyImpl::register_category(std::string const& id,
        std::string const& title,
        std::string const &icon,
        CategoryRenderer const& renderer_template)
{
    return upstream_->register_category(id, title, icon, renderer_template);
}

Category::SCPtr BufferedSearchReplyImpl::register_category(std::string const& id,
        std::string const& title,
        std::string const &icon,
        CannedQuery const &query,
        CategoryRenderer const& renderer_template)
{
    upstream_->register_category(id, title, icon, query, renderer_template);
}

void BufferedSearchReplyImpl::register_category(Category::SCPtr category)
{
    upstream_->register_category(category);
}

Category::SCPtr BufferedSearchReplyImpl::lookup_category(std::string const& id)
{
    return upstream_->lookup_category(id);
}

bool BufferedSearchReplyImpl::push(unity::scopes::experimental::Annotation const& annotation)
{
    return upstream_->push(annotation);
}

bool BufferedSearchReplyImpl::push(unity::scopes::CategorisedResult const& result)
{
    return upstream_->push(result); //FIXME
}

bool BufferedSearchReplyImpl::push(unity::scopes::Filters const& filters, unity::scopes::FilterState const& filter_state)
{
    return upstream_->push(filters, filter_state);
}

void BufferedSearchReplyImpl::finished()
{
    upstream_->finished(); // ???
}

void BufferedSearchReplyImpl::error(std::exception_ptr ex)
{
    upstream_->error(ex); // ???
}

void BufferedSearchReplyImpl::info(OperationInfo const& op_info)
{
    upstream_->info(op_info); // ???
}

std::string BufferedSearchReplyImpl::endpoint()
{
    return upstream_->endpoint();
}

std::string BufferedSearchReplyImpl::identity()
{
    return upstream_->identity();
}

std::string BufferedSearchReplyImpl::target_category()
{
    return upstream_->target_category();
}

int64_t BufferedSearchReplyImpl::timeout()
{
    return upstream_->timeout();
}

std::string BufferedSearchReplyImpl::to_string()
{
    return upstream_->to_string();
}

}

}

}
