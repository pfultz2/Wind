

#ifndef WIND_GUARD_WIND_H
#define WIND_GUARD_WIND_H

namespace wind {

template<class Iterator>
struct range
{
    Iterator first, last;

    range(Iterator first, Iterator last)
    : first(first), last(last)
    {}

    bool empty() const
    {
        return first == last;
    }

    auto&& front() const
    {
        return *first;
    }

    range pop_front() const
    {
        Iterator next = first;
        return range(++next, last);
    }

    range pop_all() const
    {
        return range(last, last);
    }

    Iterator begin() const
    {
        return first;
    }

    Iterator end() const
    {
        return last;
    }
};

template<class Iterator>
range<Iterator> make_range(Iterator start, Iterator last)
{
    return range<Iterator>(start, last);
}

template<class Range, class F>
struct parser_result
{
    Range range;
    F result;

    parser_result(Range r, F f)
    : range(r), result(f)
    {}
};

template<class Range, class F>
parser_result<Range, F> make_parser_result(Range r, F f)
{
    return parser_result<Range, F>(r, f);
}

template<class F>
struct parser;

template<class F>
parser<F> make_parser(F f)
{
    return parser<F>(std::move(f));
}


template<class Predicate>
auto parse_if(Predicate p)
{
    return make_parser([=](auto r)
    {
        auto first = r.begin();
        auto last = r.begin();
        if(last != r.end() && p(*last)) last++; 
        return make_parser_result(make_range(last, r.end()), [=](auto f)
        {
            f(*first);
        });
    });
}

template<class Predicate>
auto parse_while(Predicate p)
{
    return make_parser([=](auto r)
    {
        auto first = r.begin();
        auto last = r.begin();
        while(last != r.end() && p(*last)) last++; 
        return make_parser_result(make_range(last, r.end()), [=](auto f)
        {
            f(make_range(first, last));
        });
    });
}

template<class F>
struct parser : F
{
    template<class P>
    parser(P p) : F(std::move(p))
    {}

    template<class Action>
    auto operator[](Action a) const
    {
        return make_parser([=, self=*this](auto r)
        {
            auto result = self(r);
            return make_parser_result(result.range, [=](auto f) 
            { 
                return result.result([=](auto... xs)
                {
                    return f(a(xs...));
                }); 
            });
        });
    }

    template<class NextParser>
    auto operator >>(NextParser p) const
    {
        return make_parser([=, self=*this](auto r)
        {
            auto result1 = self(r);
            auto result2 = p(result1.range);
            return make_parser_result(result2.range, [=](auto f)
            {
                result1.result([=](auto... xs)
                {
                    result2.result([=](auto... ys)
                    {
                        f(xs..., ys...);
                    });
                });
            });
        };
    }

    template<class NextParser>
    auto operator |(NextParser p) const
    {
        // TODO
        return make_parser([=, self=*this](auto r)
        {
            // decltype(r) final_range;
            auto result1 = self(r);
            if (result1.range == r)
            {
                auto result2 = p(r);
            }
            return make_parser_result(result2.range, [=](auto f)
            {
                if (result1.range == r) result2.result(f);
                else result1.result(f);
            });
        };
    }

};

}

#endif
